import facebook
import time
import os
import urllib2
import urllib
import json
import redis
import paho.mqtt.client as mqtt
import serial


r = redis.Redis()
client = mqtt.Client()

client.connect("localhost", 1883, 0)

strPort1 = '/dev/ttyACM0'

ser = serial.Serial(strPort1, 115200)


def get_app_access_token(app_id, app_secret):
    """Get the access_token for the app.

    This token can be used for insights and creating test users.

    @arg app_id :type string :desc retrieved from the developer page
    @arg app_secret :type string :desc retrieved from the developer page

    Returns the application access_token.

    """
    # Get an app access token
    args = {'grant_type': 'client_credentials',
            'client_id': app_id,
            'client_secret': app_secret}

    f = urllib2.urlopen("https://graph.facebook.com/oauth/access_token?" +
                              urllib.urlencode(args))

    try:
    	data = f.read()
    	print data
        result = json.loads(data)['access_token']

    finally:
        f.close()

    return result


APP_ID = '126161218103081'
APP_SECRET = '495b13ba53986b8e533109bc6531c9ec'
token = get_app_access_token(APP_ID, APP_SECRET)
print token

#token = 'a60d0e410d946567a4808048a9f862dd'
#token = 'EAACEdEose0cBANpVK5LUvX6kkmJi6HcQ7ZALZBRxei3TCjMiKl7gLYTMCaqkoJ5dSxrL6EMJzLZCUgdsINPbMbGM2ejSpgkXRA3BOrZAycRkGnGsjlb3Mu4HQxYmZCF9SFO8lpHBEBj8m3LB87uBhdiK1W4lTN4YB52fgBed8XK3cYA59YfbNFvyrbARRy2cZD'
graph = facebook.GraphAPI(access_token=token, version='2.7')

likes = 0


def get_likes():
	global likes

	page = graph.get_object(id='merekamakerspace',fields='fan_count')
	#page = graph.get_object(id='1839595709654407',fields='fan_count')
	#page = graph.get_object(id='bijibijiinitiative',fields='fan_count')
	
	new_likes = int(page['fan_count'])
	
	if new_likes != likes:
		likes = new_likes
		localtime = time.asctime( time.localtime(time.time()) )
		say_str = localtime + " " + str(likes) + " likes"
		print say_str
		r.set('mereka:likes', likes)
		r.publish('topic:likes', likes)
		client.publish("mereka/likes", likes)
		ser.write(str(likes))
		#os.system(say_str)

while 1:
	get_likes()
	time.sleep(5)
	
