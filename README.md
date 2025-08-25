the official mestastic simulator doesnt work very well so i tried to put it into the docker container from my qemu & vice emulator project



this one i ran up to 200, its right on the edge of channel overutilisation with default settings maybe 300 is the max on a single default channel setting and idle clients

```
for i in {1..200} ;do     xterm -T "Node 440$i" -e /home/user/Meshtasticator/Meshtasticator-device/.pio/build/native/program --port 440$i -e &     sleep 1; done
```

<img width="382" height="912" alt="Screenshot 2025-08-24 at 6 40 48 PM" src="https://github.com/user-attachments/assets/707ee810-f19a-4c5a-9c4e-964f2eeb9abd" />


```
(venv) user@cfaa3940fc8d:/meshpy$ meshtastic --host 127.0.0.1:4403 --set mqtt.encryption_enabled false
Connected to radio
Set mqtt.encryption_enabled to false
Writing modified preferences to device
Writing mqtt configuration to device
```


also working on an mqtt broker to simulate forwarding messages between pools of different meshes 


```
(venv) user@cfaa3940fc8d:/meshpy$ gcc -std=c89 -Wall relay.c -o mqttrelay
(venv) user@cfaa3940fc8d:/meshpy$ ./mqttrelay 
Minimal MQTT broker running on port 1883
[CONNECT] New client connected, slot 0
[CONNECT] Client slot 0 sent CONNECT
[CONNACK] Sent to client slot 0
[CONNECT] New client connected, slot 1
[CONNECT] Client slot 1 sent CONNECT
[CONNACK] Sent to client slot 1
```

missing quite a bit and needs work
