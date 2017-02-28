/* Web Server (developed with express) that integrate the MQTT broker and the MQTT subscriber.
Run mongodb with 'mongod' command before to run the server!

Open http://localhost:3000/grafico to show the data.
*/
var mqtt = require('mqtt')
var express = require('express');
var app = express();
var mosca = require ('mosca')
var MongoClient = require('mongodb').MongoClient;
var path = require('path');

app.use(express.static(path.join(__dirname, 'public')));


// **************** MongoDB ****************
var mongodbMosca = {
  type: 'mongo',
  url: 'mongodb://localhost:27017/mqtt',
  pubsubCollection: 'moscaLog',
  mongo: {}
};

var mongodbsettings = {
  type: 'mongo',
  url: 'mongodb://localhost:27017/mqtt',
  pubsubCollection: 'sensor',
  mongo: {}
};


// **************** Metodi richiamabili dal browser ****************

app.get('/grafico', function (req, res){
	 res.sendFile(path.join(__dirname+'/grafico.html'));	
});


http://localhost:3000/
app.listen(3000, function () {
  console.log('Example app listening on port 3000!');
});


//methot to get the last sensor value received
app.get('/getLastValue', function (req, res) {
	console.log("\nCalled /getLastValue");
	MongoClient.connect(mongodbsettings.url, function(err, db) {
		if(err){
			console.log("Error during the connection to the database: "+err.message);
		}else{
			//console.log('Connessione stabilita to', mongodbsettings.url);
			var collection = db.collection('sensor');
			collection.find().sort({"_id":-1}).limit(1).toArray(function(err, lastSensorValue){
				if(err){
					console.log("Error: "+err.message);
				}else{
					console.log("lastSensorValue: "+JSON.stringify(lastSensorValue));
					res.send(lastSensorValue[0]);
				}
				db.close();
			});
		}
	});
});



//methot to get all the sensor value (http://localhost:3000/getAllValue)
app.get('/getAllValue', function (req, res){
	console.log("\nCalled /getAllValue");
	MongoClient.connect(mongodbsettings.url, function(err, db) {
		if(err){
			console.log("Error during the connection to the database: "+err.message);
		}else{
			//console.log('Connessione stabilita to', mongodbsettings.url);
			var collection = db.collection('sensor');
			collection.find(/*{value:'12'}*/).toArray(function(err, allSensorValue) {
				if(err){
					console.log("Errore"+err.message);
				}else{//allSensorValue
					console.log("allSensorValue: "+JSON.stringify(allSensorValue));
					res.send(allSensorValue);
				}
				db.close();
			});
		}
	});
});

// **************** Fine metodi richiamabili dal browser ****************




// **************** MOSCA CONFIGURATION - INIZIO CODICE BROKER MQTT ****************

	var settings = {
	  port: 1122,
    backend: mongodbMosca,
	  persistence: mosca.persistence.Memory
	};

	var server = new mosca.Server(settings, function() {
	  console.log('Mosca Server is up and running')
	});


	server.on('clientConnected', function(client) {
		console.log('client connected', client.id);
	});

	// fired when a message is received
	server.on('published', function(packet, client) {
	  console.log('Published', packet.payload);
	});

	server.on('ready', setup);

	// fired when the mqtt server is ready
	function setup() {
	  console.log('Mosca server is up and running');
}
// **************** MOSCA CONFIGURATION - FINE CODICE BROKER MQTT ****************

/*connect to the server*/
client = mqtt.createClient(1122, 'localhost');

/*subscribe to a topic named 'MIO_TOPIC'*/
client.on('connect', function() { // When connected
	client.subscribe('topicDemo');

});

client.on("message", function(topic, payload,packet) {
	console.log("PACKET RECEIVED: "+packet.payload.toString());
	//example of a packet received from Arduino: { "gatewayId":"11223344AABBCCDD", "nodeId":1, "sensorId":1, "sensorValue":"14"}
	var json = JSON.parse(packet.payload);
	var gatewayId=json.gatewayId;
	var nodeId=json.nodeId;
	var sensorType= json.sensorId;
	var sensorValue= json.sensorValue;		//sensor value
	var now = (new Date()).getTime();
	lastSensorValue=sensorValue;		
	console.log("\tGateway Id: "+gatewayId);
	console.log("\tNode Id: "+nodeId);
	console.log("\tSensor Type: "+sensorType);
	console.log("\tSensor value: "+sensorValue);

    MongoClient.connect(mongodbsettings.url, function(err, db) {
		if(err) {
			console.log("Error during the connection to the database: "+err.message);
		}
		else {
			var sensorCollection = db.collection('sensor');
			var document = {'gatewayId':gatewayId,'nodeId':nodeId,'type':sensorType, 'value': sensorValue, 'timestamp':now};
			sensorCollection.insert(document, {w:1}, function(err, result) {
				if (err){
					console.warn("Errore nell'inserimento nel database: "+err.message);  // returns error if no matching object found
				}else{
					console.log("Data stored into the database: "+JSON.stringify(result));
				}
				db.close();
			 });

		}
  });
});


console.log('Client started...');
