#!/bin/bash

let n=$(service iptables status | grep -ce "Stopping .*")
let z=0
if [ "${n}" -eq "${z}" ]
then
echo "the firewall is working"
service iptables start
service iptables stop
else 
echo "the firewall is stopped"
fi

make

cp ./httpServerApp.out /home/liuhualin/Documents/httpServerApp.out

/home/liuhualin/Documents/httpServerApp.out
