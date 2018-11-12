# linux-caller-id

Phone calling identification and blocking in Linux

Linux Caller Id is a tiny program that monitors incoming phone calls (a modem is obviously required) and tells you the caller phone number. It's written in Qt/C++.

You can specify which numbers will be blocked (telemarketers, etc...) and the program will "block" the caller. Well, not really block, when a blocked number calls the program will answer and then hangup. I hope this will make the caller think the number is wrong or disconnected.

The USB modem I used is a Trendnet TFM-561U, so the default values in the modem configuration page are those that work for me. If you have another modem please send me the values that work for you or add them to the wiki.

Detailed information is available at https://code.google.com/archive/p/linux-caller-id/
