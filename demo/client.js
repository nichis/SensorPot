/*eseguendo questo file invio un messaggio sul topic MIO_TOPIC sul quale è in ascolto il mio server
sto quindi simulando l'invio di un valore del sensore da Arduino.*/

var mqtt = require('mqtt')

/*connect to the server*/
var client  = mqtt.connect({ host: 'localhost', port: 1122 });

client.on('connect', function() { // When connected

  // publish a message sul topic: "MIO_TOPIC"
  var message={ "gatewayId":"11223344AABBCCDD", "nodeId":1, "sensorId":1, "sensorValue":"14"};	//scelgo JSON per il formato di invio dei dati MQTT
  client.publish('topicDemo', JSON.stringify(message), function() {
    console.log("Message is published");
    client.end(); // Close the connection when published
  });
});
