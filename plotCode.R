library(ggplot2)

# 50 
points_list <- list(c(1, 1.49585e-05), c(4, 5.0312e-05), c(8, 8.86455e-05), c(16, 0.000177855), c(32, 0.000375271),c(64,0.000750833),c(128,0.00152231))
points_df <- data.frame(
  x = sapply(points_list, function(point) point[1]),
  y = sapply(points_list, function(point) point[2])
)

ggplot(data = points_df, aes(x = x, y = y)) +
  geom_point(color = "blue", size = 3) +
  geom_line(color = "red", size = 1) +
  geom_segment(aes(x = x, xend = x, y = 0, yend = y), 
               linetype = "dashed", color = "gray") + 
  geom_segment(aes(x = 0, xend = x, y = y, yend = y), 
               linetype = "dashed", color = "gray") + 
  geom_text(aes(x = x, y = 0, label = round(x, 1)), 
            vjust = 1.5, size = 3, color = "black") +
  geom_text(aes(x = 0, y = y, label = format(y, scientific = TRUE, digits = 2)), 
            hjust = -0.3, size = 3, color = "black") +
  ggtitle("50 Dimension") +
  xlab("Amount of Thread") +
  ylab("Time (Seconds)") +
  theme_minimal() +
  theme(
    plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
    axis.title.x = element_text(size = 12, face = "bold"),
    axis.title.y = element_text(size = 12, face = "bold"),
    axis.text = element_text(size = 10)
  )

# 100
points_list <- list(c(1, 5.9917e-05), c(4, 6.3479e-05), c(8, 9.4229e-05), c(16, 0.000187146), c(32, 0.000372791),c(64,0.00075848),c(128,0.00152296))
points_df <- data.frame(
  x = sapply(points_list, function(point) point[1]),
  y = sapply(points_list, function(point) point[2])
)

ggplot(data = points_df, aes(x = x, y = y)) +
  geom_point(color = "blue", size = 3) +
  geom_line(color = "red", size = 1) +
  geom_segment(aes(x = x, xend = x, y = 0, yend = y), 
               linetype = "dashed", color = "gray") + 
  geom_segment(aes(x = 0, xend = x, y = y, yend = y), 
               linetype = "dashed", color = "gray") + 
  geom_text(aes(x = x, y = 0, label = round(x, 1)), 
            vjust = 1.5, size = 3, color = "black") +
  geom_text(aes(x = 0, y = y, label = format(y, scientific = TRUE, digits = 2)), 
            hjust = -0.3, size = 3, color = "black") +
  ggtitle("100 Dimension") +
  xlab("Amount of Thread") +
  ylab("Time (Seconds)") +
  theme_minimal() +
  theme(
    plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
    axis.title.x = element_text(size = 12, face = "bold"),
    axis.title.y = element_text(size = 12, face = "bold"),
    axis.text = element_text(size = 10)
  )

# 500
points_list <- list(c(1, 0.00152637), c(4, 0.000457479), c(8, 0.00045775), c(16, 0.000470625), c(32, 0.000511392),c(64,0.000833583),c(128,0.00159069))
points_df <- data.frame(
  x = sapply(points_list, function(point) point[1]),
  y = sapply(points_list, function(point) point[2])
)

ggplot(data = points_df, aes(x = x, y = y)) +
  geom_point(color = "blue", size = 3) +
  geom_line(color = "red", size = 1) +
  geom_segment(aes(x = x, xend = x, y = 0, yend = y), 
               linetype = "dashed", color = "gray") + 
  geom_segment(aes(x = 0, xend = x, y = y, yend = y), 
               linetype = "dashed", color = "gray") + 
  geom_text(aes(x = x, y = 0, label = round(x, 1)), 
            vjust = 1.5, size = 3, color = "black") +
  geom_text(aes(x = 0, y = y, label = format(y, scientific = TRUE, digits = 2)), 
            hjust = -0.3, size = 3, color = "black") +
  ggtitle("500 Dimension") +
  xlab("Amount of Thread") +
  ylab("Time (Seconds)") +
  theme_minimal() +
  theme(
    plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
    axis.title.x = element_text(size = 12, face = "bold"),
    axis.title.y = element_text(size = 12, face = "bold"),
    axis.text = element_text(size = 10)
  )
#1000
points_list <- list(c(1, 0.00615265), c(4, 0.00169456), c(8, 0.00151502), c(16, 0.00150579), c(32,  0.00158002),c(64,0.00186902),c(128,0.00254665))
points_df <- data.frame(
  x = sapply(points_list, function(point) point[1]),
  y = sapply(points_list, function(point) point[2])
)

ggplot(data = points_df, aes(x = x, y = y)) +
  geom_point(color = "blue", size = 3) +
  geom_line(color = "red", size = 1) +
  geom_segment(aes(x = x, xend = x, y = 0, yend = y), 
               linetype = "dashed", color = "gray") + 
  geom_segment(aes(x = 0, xend = x, y = y, yend = y), 
               linetype = "dashed", color = "gray") + 
  geom_text(aes(x = x, y = 0, label = round(x, 1)), 
            vjust = 1.5, size = 3, color = "black") +
  geom_text(aes(x = 0, y = y, label = format(y, scientific = TRUE, digits = 2)), 
            hjust = -0.3, size = 3, color = "black") +
  ggtitle("1000 Dimension") +
  xlab("Amount of Thread") +
  ylab("Time (Seconds)") +
  theme_minimal() +
  theme(
    plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
    axis.title.x = element_text(size = 12, face = "bold"),
    axis.title.y = element_text(size = 12, face = "bold"),
    axis.text = element_text(size = 10)
  )


#10000
points_list <- list(c(1, 0.628312), c(4, 0.166367), c(8, 0.129171), c(16, 0.129042), c(32,  0.128808),c(64,0.128999),c(128,0.131718))
points_df <- data.frame(
  x = sapply(points_list, function(point) point[1]),
  y = sapply(points_list, function(point) point[2])
  )

  ggplot(data = points_df, aes(x = x, y = y)) +
    geom_point(color = "blue", size = 3) +
    geom_line(color = "red", size = 1) +
    geom_segment(aes(x = x, xend = x, y = 0, yend = y), 
                 linetype = "dashed", color = "gray") + 
    geom_segment(aes(x = 0, xend = x, y = y, yend = y), 
                 linetype = "dashed", color = "gray") + 
    geom_text(aes(x = x, y = 0, label = round(x, 1)), 
              vjust = 1.5, size = 3, color = "black") +
    geom_text(aes(x = 0, y = y, label = format(y, scientific = TRUE, digits = 2)), 
              hjust = -0.3, size = 3, color = "black") +
    ggtitle("10000 Dimension") +
    xlab("Amount of Thread") +
    ylab("Time (Seconds)") +
    theme_minimal() +
    theme(
      plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
      axis.title.x = element_text(size = 12, face = "bold"),
      axis.title.y = element_text(size = 12, face = "bold"),
      axis.text = element_text(size = 10)
    )


  

  points_list <- list(
    list(c(1, 1.49585e-05), c(4, 5.0312e-05), c(8, 8.86455e-05), c(16, 0.000177855), c(32, 0.000375271), c(64, 0.000750833), c(128, 0.00152231), "50"),
    list(c(1, 5.9917e-05), c(4, 6.3479e-05), c(8, 9.4229e-05), c(16, 0.000187146), c(32, 0.000372791), c(64, 0.00075848), c(128, 0.00152296), "100"),
    list(c(1, 0.00152637), c(4, 0.000457479), c(8, 0.00045775), c(16, 0.000470625), c(32, 0.000511392), c(64, 0.000833583), c(128, 0.00159069), "500"),
    list(c(1, 0.00615265), c(4, 0.00169456), c(8, 0.00151502), c(16, 0.00150579), c(32, 0.00158002), c(64, 0.00186902), c(128, 0.00254665), "1000"),
    list(c(1, 0.628312), c(4, 0.166367), c(8, 0.129171), c(16, 0.129042), c(32, 0.128808), c(64, 0.128999), c(128, 0.131718), "10000")
  )
  
  points_df <- do.call(rbind, lapply(points_list, function(dim_data) {
    dimension <- tail(dim_data, 1)[[1]]  
 
    points <- head(dim_data, -1)  
    
   
    do.call(rbind, lapply(points, function(point) {
      data.frame(x = point[1], y = point[2], dimension = dimension)
    }))
  }))

  points_df$dimension <- factor(points_df$dimension, levels = c("50" ,"100", "500", "1000", "10000"))
  

  ggplot(data = points_df, aes(x = x, y = y, color = dimension, group = dimension)) +
    geom_point(size = 3) + 
    geom_line(size = 1) +   
    ggtitle("Performance across Dimensions") +
    xlab("Amount of Thread") +
    ylab("Time (Seconds)") +
  scale_y_log10() +
    theme_minimal() +
    theme(
      plot.title = element_text(hjust = 0.5, size = 14, face = "bold"), 
      axis.title.x = element_text(size = 12, face = "bold"),
      axis.title.y = element_text(size = 12, face = "bold"),
      axis.text = element_text(size = 10)
    ) +
    scale_color_manual(values = c("50" = "orange" , "100" = "red", "500" = "blue", "1000" = "green", "10000" = "purple")) +
    labs(color = "Dimension")
  









