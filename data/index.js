// Global Variables
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
        
// Initialize On Page Load
window.addEventListener('load', onLoad);
function onLoad(event) {
    // Initialize Global Variables
    websocket = new WebSocket(gateway);
    document.body.style.height = /Mobi/i.test(window.navigator.userAgent)? 
        window.innerHeight+'px': '100vh'
    
    // Initialize Handlers
    webSocketHandlers();
    eventHandlers();
}

// WebSocket Handling
function webSocketHandlers() {
    // On WebSocket Close 
    websocket.onclose = function() {
        setTimeout(webSocketHandlers, 2000)
    }

    // On WebSocket Message 
    websocket.onmessage = function(event) {
        var data = JSON.parse(event.data);
console.log('Recieved:',data);
        var elements = document.getElementsByTagName('*');
        var messages = {
            'test': 'Testing Calibration\n- steering...\n- throttle...\n- done',
            'save': 'Saving Calibration\n- MC values stored\n- MC values updated',
            'throttle': 'Updating Max Throttle\n- max throttle set to '+data.max, 
            'ai_on': 'AI Mode\n- launching AI\n- AI active',    
            'ai_off': 'Manual Mode\n- launching manual\n- manual active',    
            'erase': 'Erasing Records\n- erased '+data.n+' records',
            'eStop_on': 'EMERGENCY STOP !!!\n- killing throttle\n- centering steering',
            'eStop_off': 'Resuming Services\n- program resumed',
            'pid': 'Updating PID Values\n- PID values updated'                         
        }        
        
        // Update Document Values
        data.event = "Status: "+messages[data.event];
        for(var id in data)  {
            elements[id].value = data[id];

            // Trigger Slider Update
            if(elements[id].type == 'range') {
                elements[id].dispatchEvent(new Event('input'));
            }
        }
    }     
}

// Event Handlers
function eventHandlers() {
    // Number Inputs
    for(var input of document.querySelectorAll('[type=number]')) {
        input.addEventListener('input', function() {
            // Limit Length
            if (this.value.length > this.max) {
                this.value = this.value.slice(0, this.max);
            }           
        });
    }
    
    // Input Sliders
    for(var input of document.querySelectorAll('[type=range]')) {
        input.addEventListener('input', function() {
            // Update TrackS
            var gradient = '#70FEE8 '+this.value+'%, #091d36 '+this.value+'%';
            this.style.background = 'linear-gradient(90deg, '+gradient+')';                     
        });
    }

    // Message Buttons
    for(var button of document.querySelectorAll('[data-form]')) {
        button.addEventListener('click', function(){
            // Create Message
            var json = {'event':this.id};
            var data = document.querySelectorAll(this.dataset.form);
            for(var entry of data) {
                json[entry.id] = entry.value;
            }
            
            // Send Message
            console.log('Sent:',json);
            navigator.vibrate(20);
            websocket.send(JSON.stringify(json));
        });
    }

    // Resize Buttons
    for(var button of document.querySelectorAll('[data-div]')) {
        button.addEventListener('click', function(){
            // Toggle Size -- temp multiple div for #Switch
            this.innerText = this.innerText=='−'? '+': '−';
            var data = document.querySelectorAll(this.dataset.div);
            for(var div of data) {
                div.className = div.className=='min'? 'max': 'min';
            }    

            // Temp Donkey/Ros Switcher
            if(this.id='switch') {
                var program = document.getElementById('program'); 
                program.innerText = program.innerText=='DONKEY'? 'ROS': 'DONKEY';
            }
        });
    }
}