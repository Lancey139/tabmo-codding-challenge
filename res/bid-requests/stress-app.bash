#!/bin/bash

for i in {1..400}
do
   echo -e "Itération $i"
   echo -e "\nRequete 1 "
   curl -d "@1.json" -H "Content-Type: application/json; charset=utf-8"  -X GET http://127.0.0.1:8080/rtb
   echo -e "\nRequete 2 "
   curl -d "@2.json" -H "Content-Type: application/json; charset=utf-8"  -X GET http://127.0.0.1:8080/rtb
   echo -e "\nRequete 3 "
   curl -d "@3.json" -H "Content-Type: application/json; charset=utf-8"  -X GET http://127.0.0.1:8080/rtb
done


