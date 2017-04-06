module.exports = [
  {
    "type": "heading",
    "defaultValue": "App Configuration"
  },
  {
    "type": "text",
    "defaultValue": "Settings"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        "type": "color",
        "messageKey": "BackgroundColor",
        "defaultValue": "0xFFFFFF",
        "label": "Background Color"
      },
      {
        "type": "color",
        "messageKey": "TextColor",
        "defaultValue": "0x000000",
        "label": "Text Color"
      },
      {
        "type": "radiogroup",
        "messageKey": "Bluetooth",
        "label": "Bluetooth Disconnect Icon",
        "defaultValue": "Black",
        "options": 
        [
          { 
            "label": "Black", 
            "value": true
          },
          { 
            "label": "White", 
            "value": false
          }
        ]
      },
      {
        "type": "slider",
        "messageKey": "Weather_Fetch",
        "defaultValue": 15,
        "label": "Weather Fetch Frequency - every x minutes",
        "min": 5,
        "max": 60,
        "step": 5
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Battery Bar"
      },
      {
        "type": "radiogroup",
        "messageKey": "Battery",
        "label": "Color",
        "defaultValue": "Classic",
        "options": [
          { 
            "label": "Classic", 
            "value": true
          },
          { 
            "label": "Solid", 
            "value": false
          }
        ]
      },
      {
        "type": "toggle",
        "messageKey": "direction",
        "label": "Centered",
        "defaultValue": false
      }
    ]
  },
  {
    "type": "section",
    "items": 
    [
      {
        "type": "heading",
        "defaultValue": "More Settings"
      },
      {
        "type": "toggle",
        "messageKey": "Twenty_Four_Hour_Format",
        "label": "24 Hour Time",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "Celsius",
        "label": "Celsius",
        "defaultValue": false
      }
    ]
  },
  {
    "type": "section",
    "items": 
    [
      {
        "type": "heading",
        "defaultValue": "Step Goal"
      },
      {
        "type": "slider",
        "messageKey": "Step_Goal",
        "defaultValue": 0,
        "label": "Step Goal",
        "min": 0,
        "max": 20000,
        "step": 500,
        "description": "Leave 0 for no goal"
      },
      {
        "type": "color",
        "messageKey": "Goal_Color",
        "defaultValue": "0x000000",
        "label": "Step Goal Color",
        "description": "What color do you want the numbers to turn after hitting your goal?"
      },
      {
        "type": "toggle",
        "messageKey": "goalvibe",
        "label": "Vibrate",
        "defaultValue": false
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];