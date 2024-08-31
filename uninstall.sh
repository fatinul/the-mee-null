#!/bin/bash

start_line=$(grep -n "THE-MEE-NULL" ~/.bashrc | cut -d: -f1)
end_line=$(grep -n "END" ~/.bashrc | head -n1 | cut -d: -f1)

echo "Removing section from .bashrc..."
sed -i "${start_line},${end_line} d" ~/.bashrc

read -p "Remove the-mee-null binary? (y/n): " response
response=${response,,}

if [ "$response" = "y" ] || [ -z "$response" ]; then
  echo "Removing the-mee-null binary..."
  sudo rm /usr/local/bin/the-mee-null
else
  echo "Skipping removal of the-mee-null binary."
fi

echo "Thank you for checking out my program!"
