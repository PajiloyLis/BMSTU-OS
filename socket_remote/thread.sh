#!/bin/bash

for ((i=1; i<=3000; i++)); do
    gnome-terminal -- bash -c "./thread_client"
done