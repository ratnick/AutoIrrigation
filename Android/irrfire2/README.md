# FireSmartLamp-Android
Android app for controlling a Smart Lamp. It's the Android client of this Smart Lamp project https://github.com/andriyadi/FireSmartLamp.
Make sure that Smart Lamp project is deployed successfully first before running this app.

It's not only for controlling. Telemetry data (e.g wattage) sent from the device is also displayed on the app in realtime. 
Latest parameters (e.g switch on/off) are also synced between the app and device, so that you'll know whether the lamp is currently on or off. 
And as the bonus, it's quite good-looking app :P

Make sure to take a look [this deck](http://www.slideshare.net/andri_yadi/firebase-54159652) for more stories behind this app.


##Architecture
This app leverages Firebase in order to control the smart lamp remotely via internet. By using Firebase as the backend, the smart lamp doesn't need to have IP public, it just needs to be connected to internet via router or 3G/4G modem.

![Architecture](https://github.com/andriyadi/FireSmartLamp-Android/blob/master/Architecture.jpg)


##Run the app
If you already deployed that Smart Lamp project, you should already have Firebase app. Make note of Firebase app name (or project id).

Then in `DeviceActivity.java` file, on line 50, change `[YOUR_OWN_FIREBASE_APP]` to your Firebase app name/project id retrieved above.

If the app run successfully, you'll have something like this:

![Screen](https://github.com/andriyadi/FireSmartLamp-Android/blob/master/ScreenCap.png)

Notice that the app will first try to retrieve latest device object from Firebase and apply `parameters` object values to related controls (buttons and progress bar).
By that, if for example, the smart lamp is currently ON, the switch button will be ON as well. Then tap on it once will make the lamp OFF.

Sliding progress bar at the bottom is supposed to change the actual brightness/dim level of the lamp. But the implementation of dimming is not yet done on the Smart Lamp side. Maybe you wanna do it? Please do :)

That's it. Enjoy.

##Contact me

Andri Yadi

* Email: an dot dri at me dot com
* Twitter: http://twitter.com/andri_yadi
* LinkedIn: https://www.linkedin.com/in/andriyadi

For professional services, you can contact my company: office at dycode dot com

##Using it
You're welcome to use it in your projects and make any changes. Just please keep the acknowledgement.

##License
Copyright (c) 2015 Andri Yadi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


