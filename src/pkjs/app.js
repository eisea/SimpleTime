var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);


var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var myAPIKey = '675c5b3a8d1ddbc615ac01726257978e';
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
  pos.coords.latitude + '&lon=' + pos.coords.longitude + '&units=imperial&appid=' + myAPIKey;
  
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

     
      var temperature = Math.round(json.main.temp);
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].main;
      console.log('Conditions are ' + conditions);
      if (conditions.localeCompare("Thunderstorm") === 0) {
        conditions = "T-Storm";
      }
      console.log('Conditions are ' + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': conditions
      };
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }                     
);
/*
Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://adameisenstein.com.s3-website-us-east-1.amazonaws.com/index.html';
  
  Pebble.openURL(clay.generateUrl());
  //console.log('Showing configuration page: ' + url);

  //Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e && !e.response) {
    return;
  }

  // Get the keys and values from each config item
  var dict = clay.getSettings(e.response);
  console.log(JSON.stringify(e));

  // Send settings values to watch side
  Pebble.sendAppMessage(dict, function(e) {
    console.log('Sent config data to Pebble');
  }, function(e) {
    console.log('Failed to send config data!');
    console.log(JSON.stringify(e));
    
  });
});
    
  //var configData = JSON.parse(decodeURIComponent(e.response));

  //console.log('Configuration page returned: ' + JSON.stringify(configData));

Pebble.sendAppMessage({
    Twenty_Four_Hour_Format: configData.twentyFourHourFormat,
    Celsius: configData.celsius,
    //KEY_CUSTOM_LOCATION: configData.customLocation
  }, function() {
    console.log('Send successful!');
  }, function() {
    console.log('Send failed!');
});
  
});
*/