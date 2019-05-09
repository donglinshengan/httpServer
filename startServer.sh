#!/bin/bash

service iptables start
service iptables stop

make

cp ./httpServerApp.out /home/liuhualin/Documents/httpServerApp.out

/home/liuhualin/Documents/httpServerApp.out
