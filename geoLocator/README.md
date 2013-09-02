GPS-GPRS-GSM-Shield-V3.0
========================

This folder should include files required for a webserver displaying google maps and arduino sketch for communicating with the server to display the GPS coordinates of the shield. Thanks to our collaborator N4rf(b1tl34ks@gmail.com) who got this really cool stuff working

## Required Hardware

* DFRduino or similar
* GPS-GPRS-GSM-Shield-V3.0
* Battery
* Server + php

## howto

Setup your 3G configuration on the sketch, and the webserver address. (make sure it is working first, manually with AT commands and so on)
Upload the sketch, go to clear skies and wait

On Webserver files, you have to edit Google API key. Otherwise, wont work.
Setup the server and make sure it is running. Check out the the following command in order to test if data upload works.

<your-webserver-address-or-php-file-here>?latitude=-34.12345&longitude=-64.2345 //change values to try it

Then reload the main website. Another way is list the .txt files on your server to see if some new file was created with wrong date.
