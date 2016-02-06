(function() {
  loadOptions();
  submitHandler();
})();

function submitHandler() {
  var $submitButton = $('#submitButton');

  $submitButton.on('click', function() {
    console.log('Submit');

    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
  });
}

function loadOptions() {
  var $invertColorsCheckbox = $('#invertColors');
  var $timeFormatCheckbox = $('#timeFormatCheckbox');
  var $celsiusCheckbox = $('#celsiusCheckbox');

  $invertColorsCheckbox[0].checked = localStorage.invertColors === 'true';
  $timeFormatCheckbox[0].checked = localStorage.twentyFourHourFormat === 'true';
  $celsiusCheckbox[0].checked = localStorage.celsius === 'true';
  
}

function getAndStoreConfigData() {
  var $invertColorsCheckbox = $('#invertColors');
  var $timeFormatCheckbox = $('#timeFormatCheckbox');
  var $celsiusCheckbox = $('#celsiusCheckbox');

  var options = {
    invertColors: $invertColorsCheckbox[0].checked,
    twentyFourHourFormat: $timeFormatCheckbox[0].checked
    celsius: $celsiusCheckbox[0].checked
  };

  localStorage.invertColors = options.invertColors;
  localStorage.twentyFourHourFormat = options.twentyFourHourFormat;
  localStorage.celsius = options.celsius;

  console.log('Got options: ' + JSON.stringify(options));
  return options;
}

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}
