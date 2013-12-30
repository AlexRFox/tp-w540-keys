#! /bin/sh

xmodmap -pke | grep -q F13

if [ $? -eq 1 ]
then
    gen-xmodmap > ${HOME}/.Xmodmap
    xmodmap ${HOME}/.Xmodmap
fi
