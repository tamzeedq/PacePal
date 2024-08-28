# **PacePal**
A running coach that tracks your pace and helps you visualize your run using IoT, Ardiuno, Firebase,and Python.  

### Background
Over this past summer, I have taken up running as a hobby and my main form of cardio. I quickly realized the importance of maintaining pace for distance running, and started looking to buy a pace tracking watch (Apple Watch, FitBit, or other). Instead, I thought I could make something myself. Using an Arduino board with IoT support and FireBase, I created this project to track my pace while I run and used Python to plot my run data with the granularity and metrics I desired. This project was inspired by Strava.

### Technologies
- [Arduino Nano 33 IoT](https://docs.arduino.cc/hardware/nano-33-iot/)
- [Beitian BN-220](https://files.banggood.com/2016/11/BN-220%20GPS+Antenna%20datasheet.pdf) (GPS Module)
- \>= Python 3.10
- [Firebase](https://firebase.google.com/)
> Other GPS Modules may suffice as the code doesn't expect a specific module, this happens to be the one I used

# Setup

Begin by cloning the repository, then this README will go over setting up each individual component.

## Arduino 
### Required Libraries
All of the following libraries are required to run the Arduino sketch and can be downloaded through the Ardiuno IDE.

- TinyGPSPlus by Mikal Hart
- Firebase Arduino based on WiFiNINA by Mobizt
- WiFiNINA by Arduino

You'll also need to fill in the `config.h` file and replace the `xxx`s  :
```cpp
// config.h
#ifndef CONFIG_H
#define CONFIG_H

// wifi name and password
#define WIFI_SSID "xxx" 
#define WIFI_PASSWORD "xxx" 

// Firebase info
#define API_KEY "xxx"
#define DATABASE_URL "xxx" // e.g. your-database-name.firebaseio.com
#define DB_SECRET "xxx"

// The Google login used to create your Firebase Project
#define USER_EMAIL "xxx"
#define USER_PASSWORD "xxx"

#endif // CONFIG_H
```

Most of these fields you'll be able fillout after reading over the Firebase set up section. 


Schema: ....


## Firebase
This project uses Firebase's realtime database to store the run metrics. To set it up, start by going to [Firebase](https://firebase.google.com/), logging in with your Google Account, then create a new project. Then, select your project so you can see the project overview screen. We need a few key items: a **Web API Key** , **Database URL**, **Database Secret**, and a **Credential Key**. You'll need these fields for the `config.h` file for the Arduino sketch and the `.env` file for the Python visualizer.

`Web API Key`

Click the **gear icon** on the left beside **Project Overview** and go to **Project Settings**. Under **General**, scroll down to **Your Apps** and create a Web app (button with `<\>` icon) and add a Firebase SDK. In SDK config you'll see several fields, but the value labelled `apiKey` will be your `Web API Key`.

`Database URL`

You may have to create your Realtime Database first. To do so, cick **Build** > **Realtime Database**. The **Realtime Database** menu should come up under project shortcuts, click it and click **Create Database** and create it in *Test Mode*. You should now be able to see your `Database URL` with the format **https://name-1234-default-rtdb.firebaseio.com/**

`Database Secret`

Go back to **Project Settings** as when you were getting the *Web API Key*, and go to **Service Accounts**. Under **Database secret**, you can get the secret to your data base.

`Credential Key`

In the same **Service Accounts** menu as the previous step, select **Firebase Admin SDK** and click **Generate new private key**. This will download a JSON file that will serve as your credential key. Copy this file into the directory of this project.


## Visualizer
Start by creating a `.env` file of the format below, and fill in the fields with the fields you got from the previous Firebase setup step:

```bash
FIREBASE_CRED_PATH = 'firebase-credential-filename.json'
FIREBASE_DB_URL = 'https://name-1234-default-rtdb.firebaseio.com/'
```

### Virtual Environment (Optional)
Running the project in a virtual environment may be ideal to avoid conflicts.

To create an environment, clone the repo and inside the directory run the following:

```bash
python -m venv .venv
```

`.venv` can be replaced with whatever you would like to name the environment.

After creating the environment run either of the following to activate the environment:

### For Windows

```bash
.venv\Scripts\activate
```

### For Unix or Mac OS

```bash
source .venv/bin/activate
```
---


### Install Dependencies
To install the required dependencies run the following either in the project directory or in a virtual environment:

```bash
pip install -r requirements.txt
```

### Run
To run the visualizer, run the following either in the project directory or in a virtual environment:

```bash
python visualize.py
```
Or:
```bash
python3 visualize.py
```

Inside the `visualize.py` script, change the date at the bottom of the script to fetch the data from your desired day
