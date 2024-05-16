# In-class exercise 3: Help to parallelize K-NN
We need to optimize and parallelize the K-Nearest-Neighbors algorithm. We have a set of data points used for predictions and a set of input points for which we need to make these predictions. All points are in 2D, and each point in the data set has a class label (e.g., Class 0, Class 1). The algorithm is executed as follows:


1. For a point P in the input point cloud, we find the k nearest points in the data point cloud.
2. Count how many times each class appears among the k nearest neighbors. Assign the class label which appears the most to point P.
3. Repeat step 1 and step 2 for every point in the input point cloud.
4. Count the number of points in each class for the input point cloud and print.

## How to run the code

```bash
make
# run sequential implementation
./sequential_implementation
# run parallel implementation
./parallel_implementation
```
