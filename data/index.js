var websocket;
        
// Initialize Page on Load
window.onload = function() {
    // Define User WebSocket
    websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
    
    // Set Mobile Device Height
    document.body.style.height = /Mobi/i.test(window.navigator.userAgent)? 
        window.innerHeight+'px': '100vh'
    
    // Set Page Handlers
    webSocketHandlers();
    eventHandlers();
}

// WebSocket Handlers
function webSocketHandlers() {
    // On WebSocket Close 
    websocket.onclose = function() {
        setTimeout(webSocketHandlers, 2000);
    }

    // On WebSocket Message 
    websocket.onmessage = function(event) {
        var data = JSON.parse(event.data);
        var elements = document.getElementsByTagName('*');
console.log(data);
        // Define User Messages
        var messages = {
            'load': 'Loading Current Values\n- MC values updated',
            'test': 'Testing Calibration\n- steering...\n- throttle...\n- done',
            'save': 'Saving Calibration\n- MC values stored\n- MC values updated',
            'scale': 'Scaling Throttle\n- throttle scalar set to '+data.tScalar, 
            'ai': data.ai? 
                'AI Mode\n- launching AI\n- AI active':    
                'Manual Mode\n- launching manual\n- manual active',    
            'erase': 'Erasing Records\n- erased '+data.nRecords+' records',
            'eStop': data.eStop?
                'EMERGENCY STOP !!!\n- killing throttle\n- centering steering':
                'Resuming Services\n- program resumed',
            'pid': 'Updating PID Values\n- PID values updated'                         
        }        
        
        // Update Document Values
        data.event = "Status: "+messages[data.event];
        for(var id in data)  {
            elements[id].value = data[id];

            // Trigger Slider Updates
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
            // Update Tracks
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
            if(this.id == 'switch') {
                var program = document.getElementById('program'); 
                program.innerText = program.innerText=='DONKEY'? 'ROS': 'DONKEY';
            }
        });
    }
}