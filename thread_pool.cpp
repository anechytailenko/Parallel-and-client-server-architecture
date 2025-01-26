#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <algorithm>


using namespace std;

//start GLOBAL
condition_variable cv_;

mutex task_queue_mtx;

mutex file_mtx;
const int SIZE_OF_QUEUE = 20;

int getRandomNumber(int&& startNum, int&& endNum) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(startNum, endNum);
    return dis(gen);

}

// end GLOBAL

//start reserch
class Research {
public:
    static vector<double> time_thread_in_wait_state;
    static vector<double> time_until_queue_filled;
    static int amount_of_rejected_task ;

    static ofstream outFile;

    static double calculateAverage(vector<double>& vec) {
        double sum = accumulate(vec.begin(), vec.end(), 0.0);
        return sum / vec.size();
    }

    static int findMinValue(vector<double>& vec) {
        return *min_element(vec.begin(), vec.end());
    }

    static int findMaxValue(vector<double>& vec) {
        return *max_element(vec.begin(), vec.end());
    }

};
ofstream Research::outFile("research.txt", ios::app);
int Research::amount_of_rejected_task = 0;
vector<double> Research::time_thread_in_wait_state;
vector<double> Research::time_until_queue_filled;
//end research



class TaskGenerator { // maybe will be better to name Task
private:
    int id_;

public:
TaskGenerator(int id): id_(id) {}


void operator()() {
    int duration = getRandomNumber(5,10);
    this_thread::sleep_for(chrono::seconds(duration));
    cout<< "Task with id_ "<< id_<<" was executing for "<<duration<< " seconds in thread "<< this_thread::get_id()<< endl;
}

};


class TaskQueue {
private:

    queue<function<void()>> tasks_;
    atomic<int> counter_task_id ;
    atomic<bool> is_ability_add_task ;




public:
    TaskQueue(): counter_task_id(0),is_ability_add_task(true){};

    void addTaskToTaskQueue() {

        auto startTime = chrono::high_resolution_clock::now();
        bool non_first_time_calculaate_time = false;

        while(is_ability_add_task){
            if(tasks_.size() >= 20) {
                cout << "Task with id "<< counter_task_id<<" was discarded"<< endl;

                auto endTime = chrono::high_resolution_clock::now();
                auto duration = chrono::duration<double>(endTime - startTime).count();
                Research:: time_until_queue_filled.push_back(duration);
                Research:: amount_of_rejected_task ++;
                non_first_time_calculaate_time = true;

            } else {
                tasks_.emplace(TaskGenerator(counter_task_id));
                cout << "Task with id "<< counter_task_id<<" was added in the Task Queue"<< endl;

                if (non_first_time_calculaate_time) {
                    startTime = chrono::high_resolution_clock::now();
                }


            }
            counter_task_id++;
            cv_.notify_one();
            this_thread::sleep_for(chrono::milliseconds(500));

        }
    }


    function<void()> getNextExecutedTask() {
        function<void()> nextExecutedTask;
        {
            nextExecutedTask =move(tasks_.front());
            tasks_.pop();
        }

        return nextExecutedTask;
    }

    atomic<bool> isEmpty() {
        return tasks_.empty();
    }

    void stopAddTaskQueue() {
        is_ability_add_task = false;
    }







};

class ThreadPool {
private:
    vector<thread> vec_worker_thread ;
    TaskQueue& task_queue;
const int amount_of_worker_threads = 6;
    atomic<bool> is_terminate_with_active_task ;
    atomic<bool> is_terminate_without_active_task ;
    atomic<bool> is_paused;
    atomic<int> amount_of_last_active_task ;

public:

    ThreadPool(TaskQueue& task_queue): task_queue(task_queue) {
        amount_of_last_active_task =0;
        is_terminate_with_active_task = false;
        is_terminate_without_active_task = false;
        is_paused = false;
    };

    void start(){
        for (size_t i = 0; i < amount_of_worker_threads; ++i) {
            vec_worker_thread.emplace_back([this] {
                while (true) {

                    function<void()> task;
                    auto startTime = chrono::high_resolution_clock::now();
                    {
                        unique_lock<mutex> lock(task_queue_mtx);


                        cv_.wait(lock, [this] {
                            return (!task_queue.isEmpty() && !is_paused) || is_terminate_with_active_task || is_terminate_without_active_task  ;
                        });

                        if(is_terminate_with_active_task) {
                            amount_of_last_active_task+=1;
                        }


                        if ((is_terminate_with_active_task && amount_of_last_active_task > SIZE_OF_QUEUE) || is_terminate_without_active_task) {
                            return;
                        }

                        task = task_queue.getNextExecutedTask();

                    }
                    auto endTime = chrono::high_resolution_clock::now();
                    auto duration = chrono::duration<double>(endTime - startTime).count();
                    Research:: time_thread_in_wait_state.push_back(duration);

                    task();
                }
            });
        }
    }


    void stopWithExecuteActiveTask() {
        unique_lock<mutex> lock(task_queue_mtx);
        is_terminate_with_active_task = true;


    }


    void stopWithoutExecuteActiveTask() {
        unique_lock<mutex> lock(task_queue_mtx);

        is_terminate_without_active_task = true;

    }


    void pause() {
        unique_lock<mutex> lock(task_queue_mtx);
        is_paused = true;
    }

    void resume() {
        unique_lock<mutex> lock(task_queue_mtx);
        is_paused = false;
        cv_.notify_all();
    }

    ~ThreadPool() {
        for (auto& workerThread : vec_worker_thread) {
            workerThread.join();
        }
    }
};



int main() {

    TaskQueue task_queue;
    thread task_queue_thread = thread(&TaskQueue::addTaskToTaskQueue, &task_queue);

    // ThreadPool poolResearch(task_queue);
    // poolResearch.start();
    // this_thread::sleep_for(chrono::seconds(30));
    // {
    //     unique_lock<mutex> lock(file_mtx);
    //     Research::outFile << "Research\n";
    //     Research::outFile << "Average time of thread in waiting state: " << Research::calculateAverage(Research::time_thread_in_wait_state) << endl;
    //     Research::outFile << "Amount of rejected threads: " << Research::amount_of_rejected_task << endl;
    //     Research::outFile << "Max time until queue is filled: " << Research::findMaxValue(Research::time_until_queue_filled) << endl;
    //     Research::outFile << "Min time until queue is filled: " << Research::findMinValue(Research::time_until_queue_filled) << endl;
    //     Research::outFile.flush();
    // }
    // cout << "\n-------------Written in file---------------\n";


    // ThreadPool poolOne(task_queue);
    // poolOne.start();
    // this_thread::sleep_for(chrono::seconds(30));
    // cout<<"\n ----------STOP---------\n"; // after this should appear 6 executed tasks(amount of working thread)
    //
    // poolOne.stopWithoutExecuteActiveTask();

    // ThreadPool poolTwo(task_queue);
    // poolTwo.start();
    // this_thread::sleep_for(chrono::seconds(20));
    // cout<<"\n ----------STOP---------\n"; // after this should  appear 26 executed tasks(amount of working thread + size of queue)
    // poolTwo.stopWithExecuteActiveTask();

    // ThreadPool poolThree(task_queue);
    // poolThree.start();
    // this_thread::sleep_for(chrono::seconds(20));
    // cout<<"\n ----------START_PAUSE---------\n"; // after this should  appear 6 executed tasks(amount of working thread + size of queue)
    // poolThree.pause();
    // this_thread::sleep_for(chrono::seconds(30));
    // cout<<"\n ----------END_PAUSE---------\n"; // after tasks should not stop to execute
    // poolThree.resume();



    task_queue_thread.join();
    return 0;
}