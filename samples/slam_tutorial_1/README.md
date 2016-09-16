# SLAM samples

This console app illustrates the use of libRealsense and libSLam libraries to use the ZR300 camera's depth, fish eye, and IMU sensors to print out the current camera module position and pose.


## Running

Attach the ZR300 camera and invoke `./slam_sample`. As you move the camera around, you should see that the translation vector change in the tracking updates. Once medium tracking accuracy is reached, you should see messages about the occupancy map being updated.

Press any key to stop tracking and exit the sample program. An image file `trajectory.ppm` is written to the current directory. This image show a top down view of the camera trajectory on top of a representation of the occupancy map. Red indicates region of space that are "occupied" (i.e. not free space)

```

---- starting ----
Press Enter key to exit
tracking: accuracy=low,	trans=( 0.00,  0.00,  0.00)
occupancy map: creating update storage
tracking: accuracy=low,	trans=(-0.00,  0.00,  0.00)
tracking: accuracy=low,	trans=(-0.01,  0.01,  0.00)
tracking: accuracy=low,	trans=(-0.01,  0.02,  0.00)
tracking: accuracy=low,	trans=(-0.01,  0.02,  0.00)
tracking: accuracy=low,	trans=(-0.01,  0.03,  0.01)
tracking: accuracy=low,	trans=(-0.36,  0.18,  0.06)
tracking: accuracy=low,	trans=(-0.47,  0.23,  0.07)
tracking: accuracy=low,	trans=(-0.59,  0.28,  0.06)
tracking: accuracy=low,	trans=(-0.72,  0.33,  0.06)
tracking: accuracy=low,	trans=(-0.84,  0.38,  0.06)
```
:scissors: 
```
tracking: accuracy=med,	trans=(-129.74,  14.83, -113.23)
occ. map: tiles updated=879	tile_0: pos=(-129.45, -113.20) occupancy= 0%
tracking: accuracy=med,	trans=(-130.00,  14.64, -112.88)
tracking: accuracy=med,	trans=(-130.57,  14.79, -113.08)
occ. map: tiles updated=1023	tile_0: pos=(-130.00, -111.80) occupancy=79%
tracking: accuracy=med,	trans=(-131.05,  14.79, -113.05)
tracking: accuracy=med,	trans=(-131.57,  14.72, -113.01)
occ. map: tiles updated=1113	tile_0: pos=(-131.10, -112.05) occupancy=37%
saving occupancy map image: 0

--- stopping ----
```


