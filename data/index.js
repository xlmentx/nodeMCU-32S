// Global Variables
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var buttons;
var mcInputs;
var dInputs;

// Calculate Mobile Viewport
document.documentElement.style.setProperty('--vh', window.innerHeight+"px");
        
// Initialize On Page Load
window.addEventListener('load', onLoad);
function onLoad(event) {
    // Initialize Global Variables
    websocket = new WebSocket(gateway);
    buttons = document.getElementsByTagName("button");
    mcInputs = document.getElementById('calibration').getElementsByTagName("input");
    dInputs = document.getElementById('donkey').getElementsByTagName("input");

    // Initialize Handlers
    webSocketHandlers();
    eventHandlers();
}

// WebSocket Handling
function webSocketHandlers() {
    // On WebSocket Close 
    websocket.onclose = function() {setTimeout(webSocketHandlers, 2000)}

    // On WebSocket Message 
    websocket.onmessage = function(event) {
        let data = JSON.parse(event.data);
        
        // Check Message Event
        if(data.event == 'test') {
            statusUpdate("Testing MC Calibration", "- steering...\n- throttle...\n- done\n")
        }
        else if(data.event == 'save') {    
            for(var i=0; i < mcInputs.length; i++) {
                mcInputs[i].value = data[mcInputs[i].id];
            }
            statusUpdate("Saving MC Calibration", "- values stored locally\n- updating MC values\n")
        }
        else if(data.event == 'throttle') {    
            dInputs.throttle.value = data.throttle;
            statusUpdate("Updating Max Throttle", "- throttle updated") 
        }
        else if(data.event == 'ai') {    
            var type = (data.state == 'on')? 'AI':'Manual';
            buttons.ai.className = data.state;
            statusUpdate(type+" Mode", "- launching "+type+"...\n- "+type+" active")    
        }
        else if(data.event == 'erase') {    
            dInputs.n_records.value = data.n_records;
            statusUpdate("Erasing Records", "- erased "+data.n_records+" records")
        }
        else if(data.event == 'eStop') {    
            buttons.eStop.className = data.state;
            var update = (data.state == 'on')?
                ["EMERGENCY STOP !!!", "- killing throttle\n- centering steering"]:
                ["Running Donkey", "- services resumed"];
            statusUpdate(update[0], update[1]);            
        }
    }     
}

function statusUpdate(statusText, logText) {
    document.getElementById("status").innerText = statusText;
    document.getElementById("log").value = logText;    
}

// Event Handlers
function eventHandlers() {
    buttons.resize.addEventListener('click', resizeDiv.bind(buttons.resize));
    buttons.test.addEventListener('click', sendMessage.bind(buttons.test, mcInputs));
    buttons.save.addEventListener('click', sendMessage.bind(buttons.save, mcInputs));    
    buttons.ai.addEventListener('click', sendMessage.bind(buttons.ai));
    buttons.erase.addEventListener('click', sendMessage.bind(buttons.erase, [dInputs.n_records]));
    buttons.eStop.addEventListener('click', sendMessage.bind(buttons.eStop));
    dInputs.throttle.addEventListener('change', sendMessage.bind(dInputs.throttle,[dInputs.throttle]));
}

// Resize Div
function resizeDiv() { 
    var update =this.innerText=='−'? ['+','min']: ['−','max'];
    this.innerText = update[0];
    document.getElementById('calibration').className = update[1];
}

// Send JSON Message
function sendMessage(elements) { 
    var json = {'event':this.id};
    if(elements.length > 0) {
        for(var i=0; i < elements.length; i++) {
            json[elements[i].id] = elements[i].value;
        }
    }
    websocket.send(JSON.stringify(json));
}