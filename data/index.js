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
        // Get User Message
        var data = JSON.parse(event.data);
        var messages = {
            'test': 'Testing Calibration\n- steering...\n- throttle...\n- done',
            'save': 'Saving Calibration\n- MC values stored\n- MC values updated',
            'throttle': 'Updating Max Throttle\n- max throttle updated', 
            'ai_on': 'AI Mode\n- launching AI\n- AI active',    
            'ai_off': 'Manual Mode\n- launching manual\n- manual active',    
            'erase': 'Erasing Records\n- erased '+data.n+' records',
            'eStop_on': 'EMERGENCY STOP !!!\n- killing throttle\n- centering steering',
            'eStop_off': 'Resuming Program\n- services resumed',
            'pid': 'Updating PID Values\n- PID values updated'                         
        }        
        data.event = messages[data.event];
        
        // Update Document Values
        var elements = document.getElementsByTagName('*');
        for(var id in data)  {
            elements[id].value = data[id];
        }
    }     
}

// Event Handlers
function eventHandlers() {
    // Number Inputs
    for(var number of document.querySelectorAll('[type=number]')) {
        number.addEventListener('input', function() {
            // Limit Length
            if (this.value.length > this.maxLength) {
                this.value = this.value.slice(0, this.maxLength);
            }                
        });
    }
    
    // Input Sliders
    for(var slider of document.querySelectorAll('[type=range]')) {
        slider.addEventListener('input', function() {
            // Update Track
            var gradient = '#70FEE8 '+this.value+'%, #091d36 '+this.value+'%';
            this.style.background = 'linear-gradient(90deg, '+gradient+')';                
        });
    }

    // Message Buttons
    for(var btn of document.querySelectorAll('[data-form]')) {
        btn.addEventListener('click', function(){
            // Create Message
            var json = {'event':this.id};
            var data = document.querySelectorAll('#'+this.dataset.form+' input');
            for(var entry of data) {
                json[entry.id] = entry.value;
            }
            
            // Send Message
            navigator.vibrate(20);
            websocket.send(JSON.stringify(json));
        });
    }

    // Resize Buttons
    for(var btn of document.querySelectorAll('[data-div]')) {
        btn.addEventListener('click', function(){
            // Toggle Size -- temp multiple div for #Switch
            this.innerText = this.innerText=='−'? '+': '−';
            var data = this.dataset.div.split(' ');
            for(var id of data) {
                var div = document.querySelector('#'+id);
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