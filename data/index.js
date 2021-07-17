// Global Variables
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var buttons;
var mcInputs;
var dInputs;

// Initialize On Page Load
window.addEventListener('load', onLoad);
function onLoad(event) {
    // Initialize Global Variables
    websocket = new WebSocket(gateway);
    buttons = document.getElementsByTagName("button");
    mcInputs = document.getElementById('mc').getElementsByTagName("input");
    dInputs = document.getElementById('donkey').getElementsByTagName("input");
    
    // Initialize Handlers
    webSocketHandlers();
    buttonHandlers();
}

// WebSocket Handling
function webSocketHandlers() {
    // On WebSocket Close 
    websocket.onclose = function() {setTimeout(webSocketHandlers, 2000)}

    // On WebSocket Message 
    websocket.onmessage = function(event) {
        // Local Variables
        let data = JSON.parse(event.data);
        var status = document.getElementById("status");
        var log = document.getElementById("log");    

        // Test Event 
        if(data.event == 'test') {
            status.innerText = "Testing MC Calibration";
            log.value = "- Steering...\n- Throttle...\n- Done\n";
        }
        // Save Event 
        else if (data.event == 'save') {    
            status.innerText = "Saving MC Calibration";
            log.value = "- values stored locally\n- updating MC values\n"; 
            for(var i=0; i < mcInputs.length; i++) {
                mcInputs[i].value = data[mcInputs[i].id];
            }
        }
        // AI Toggle Event 
        else if (data.event == 'ai') {    
            var update = (data.state == 'on')?
                ["AI Mode", "- launching AI...\n- AI active"]:
                ["Driver Mode", "- starting manual...\n- manual active"];
            status.innerText = update[0]; 
            log.value = update[1];
            document.getElementById('ai').className = data.state;
        }
        // Erase Event     
        else if (data.event == 'erase') {    
            status.innerText = "Erasing Records"; 
            log.value = "- erased "+data.n_records+" records";
            document.getElementById('n_records').value = data.n_records;
        }
        // E-Stop
        else if (data.event == 'eStop') {    
            var update = (data.state == 'on')?
                ["EMERGENCY STOP !!!", "- killing throttle\n- centering steering"]:
                ["Running Donkey", "- resuming services"];
            status.innerText = update[0]; 
            log.value = update[1];
            document.getElementById('eStop').className = data.state;            
        }
    }     
}

// Button Handlers
function buttonHandlers() {
    buttons.resize.addEventListener('click', resizeMC.bind(buttons.resize));
    buttons.test.addEventListener('click', sendMessage.bind(buttons.test, mcInputs));
    buttons.save.addEventListener('click', sendMessage.bind(buttons.save, mcInputs));
    buttons.ai.addEventListener('click', sendMessage.bind(buttons.ai));
    buttons.erase.addEventListener('click', sendMessage.bind(buttons.erase, [dInputs.n_records]));
    buttons.eStop.addEventListener('click', sendMessage.bind(buttons.eStop));
}

// Resize MC Div
function resizeMC() {
    var update =this.innerText=='−'? ['+','min']: ['−','max'];
    this.innerText = update[0];
    document.getElementById('mc').className = update[1];
}

// Send JSON Message
function sendMessage(elements) { 
    // Create Message
    var json = {'event':this.id};
    if(elements.length > 0) {
        for(var i=0; i < elements.length; i++) {
            json[elements[i].id] = elements[i].value;
        }
    }

    // Send Message
    websocket.send(JSON.stringify(json));
}