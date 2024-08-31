#!/bin/bash

start_line=$(grep -n "THE-MEE-NULL" ~/.bashrc | cut -d: -f1)
end_line=$(grep -n "END" ~/.bashrc | head -n1 | cut -d: -f1)

sed -i "${start_line},${end_line} d" ~/.bashrc

echo "THE-MEE-NULL have been removed from the .bashrc file. Try open new terminal or 'source ~/.bashrc' to see the effect."
