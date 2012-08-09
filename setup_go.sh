#!/usr/bin/env bash
cmd=(dialog --keep-tite --menu "Select options:" 22 76 16)

options=(1 "Go plain"
         2 "Go SCTP")

choices=$("${cmd[@]}" "${options[@]}" 2>&1 >/dev/tty)

case $choices in 
  1)
    echo "Go"
    rm /usr/local/go
    ln -s /Users/olivier/Projects/MSc/go /usr/local/go;;
  2)
    echo "Go SCTP"
    rm /usr/local/go
    ln -s /Users/olivier/Projects/MSc/go-sctp /usr/local/go;;
esac

ls -l /usr/local/go

