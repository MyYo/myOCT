# myOCT
myOCT GitHub library contains multiple Matlab scripts for processing of Optical Coherence Tomography samples, scanned by Thorlab's Ganymede & Telesto’s OCT Systems.

The library contains 5 layers of scripts:

1 - LoadSave: a set of scripts that extract interferogram from OCT data files, unzip .oct files, configure AWS, and OCT chirp settings.

2 - Processing: a set of scripts that process 2D and 3D scans and return: B-scan average, Speckle Variance, Dual-Band analysis, Tif stack, and more.

3 - Demo_scripts: easy to use scripts for basic processing; require only file location as input.

4 - Testers: a set of scripts, that utilize LoadSave and Processing with parallel computing capabilities, to completely process any 2D and 3D scans using Jenkins. In addition, it contains  a set of scripts that load OCT files and check performances and imagery.

5 - AWSUtiles: a set of scripts that can start an AWS EC2 instance (worker), send data to it, run functions/processes, copy files (fast!) to your s3 Bucket, terminate the instance and more. 

This code is still at beta.

 Last updated: August 2019
