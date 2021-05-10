import time
import paho.mqtt.client as mqtt


client = mqtt.Client("test")
client.username_pw_set(username="user1",password="h8y98UyhX273X6tT")

client.connect("iotplatform.caps.in.tum.de", 1885)

arr = [1,1,1,1,2,2,2,2,1,1,1,2,2,2,1,1,1,1,2,2,2,2,2,1,1,1,1,1]
for a in arr:
	if a == 1:
		client.publish("ROOM_EVENTS_TEST", "enter")
		print("Just published " + "enter" + " to topic room_test_events")
	else:
		client.publish("ROOM_EVENTS_TEST", "leave")
		print("Just published " + "leave" + " to topic room_test_events")
		time.sleep(2)