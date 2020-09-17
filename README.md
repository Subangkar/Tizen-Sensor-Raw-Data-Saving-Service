# Raw Data Collection from Sensors of Tizen Device 
This repo contains a tizen studio project of a tizen native background service to collect raw sensor data from a tizen OS based smartwatch device.  
This service is started by by pressing the “Start” button from the native app screen of this [repo](https://github.com/Subangkar/Gear-Fit-2-Sensor-Raw-Data-Sync).  
  
[<img align="right" alt="System Diagram" src="https://i.ibb.co/xqPthDP/image.png" />](https://i.ibb.co/xqPthDP/image.png)  
  
**SENSOR DATA COLLECTION:** 
- This Logger service activates sensor periodically using timers 
- At the same time two timers are set one with 5 minutes for current recording to stop and another with 30 minutes to start the recording again 
- Once sensors are activated Data recording starts & the recording continues up to 5 minutes if watch is on hand 
- After sensors are stopped, recorded data are saved into a single CSV file for that segment with device id as part of the filename for identifying individuals uniquely   
- After each recording, the logger service checks for Wi-Fi availability and tries to upload all the csv files present locally to a remote [Heroku server](http://hr-logger.herokuapp.com/data) via Wi-Fi if it is available - Successfully uploaded files are deleted from watch storage  
  
**DATA SPECIFICATIONS:**
- Device: Samsung Gear Fit 2 Pro Smartwatch
- Sensors:
  - PPG
  - Accelerometer 
  - Gyro 
  - Pressure 
  - Gravity 
  - Activity (Stationary/Walking/Running/In Vehicle) 
- Data Collection Frequency: 10Hz
- Sample CSV columns: HR, PPG, Accelerometer_(X,Y,Z), Gyro_(X,Y,Z), Pressure, Gravity_(X,Y,Z), Activity(N/S/W/R/V), Reading UNIX Timestamp in seconds   
  
**System Requirements:**
  - Tizen SDK-2.2.1
  
**How to Build/Run:**
  - Load both this project and native app project in tizen studio
  - Link this background service with [native app](https://github.com/Subangkar/Gear-Fit-2-Sensor-Raw-Data-Sync)
  - Build and Run the native app in emulator/device
  
Tizen studio might have some issues while loading projects due to mismatch in version of tizen-manifest.xml. Then try to create new background service & native app project and replace source files to build properly.
  
<br />
  
**N.B.** This project was a part of a research work based on a tizen based smartwatch device.
