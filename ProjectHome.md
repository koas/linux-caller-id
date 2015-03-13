Linux Caller Id is a tiny program that monitors incoming phone calls (a modem is obviously required) and tells you the caller phone number. It's written in Qt/C++.

You can specify which numbers will be blocked (telemarketers, etc...) and the program will "block" the caller. Well, not really block, when a blocked number calls the program will answer and then hangup. I hope this will make the caller think the number is wrong or disconnected.

The USB modem I used is a [Trendnet TFM-561U](http://www.trendnet.com/products/proddetail.asp?prod=130_TFM-561U&cat=51), so the default values in the modem configuration page are those that work for me. If you have another modem please send me the values that work for you or add them to the wiki.

A brief guide is available in [the wiki](https://code.google.com/p/linux-caller-id/wiki/HowItWorks).

Source code is available, but if you are not a coder and just want to use the program, you can [download the executable](http://code.google.com/p/linux-caller-id/downloads/detail?name=LinuxCallerId) (sorry, I don't know how to make packages yet). The program requires libQtGui and libQtCore to be installed; in my Ubuntu box it's done by typing

sudo apt-get install libQtCore4 libQtGui4

Feel free to write me if you have any problem.