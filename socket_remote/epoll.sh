#!/bin/bash

for ((i=1; i<=3000; i++)); do
    gnome-terminal -- bash -c "./epoll_client"
done