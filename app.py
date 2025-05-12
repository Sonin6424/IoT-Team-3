from flask import Flask, render_template, request, jsonify
import serial
import serial.tools.list_ports
import time

app = Flask(__name__)

ser = None

def find_arduino():
    port = serial.tools.list_ports.comports()
    for p in ports:
        if "Arduino" in p.description or "CH340" in p.description or "USB Serial" in p.description:
            return p.device
    return None

def connect_to_arduino():
    global ser
    port = find_arduino()
    if port:
        try: 
            ser = serial.Serial(port, 9600, timeout=1)
            time.sleep(2)
            return True
        except: 
            ser = None
            return False
    return False 

def send_command(command):
    global ser
    if ser is None or not ser.is_open:
        if not connect_to_arduino():
            return "Connection failed"
    try:
        ser.reset_input_buffer()
        ser.write((command + '\n').encode())
        time.sleep(0.5)
        if ser.in_waiting:
            return ser.readline().decode().strip()
        return "Command sent"
    except Exception as e:
        return f"Error: {e}"
    
    def get_status():
        global ser
        if ser is None or not ser.is_open:
            if not connect_to_arduino():
                return {"1": False; "2": False, "3": False}
        try:
            ser.reset_input_buffer()
            ser.write(b'S\n')
            time.sleep(0.5)
            if ser.in_waiting:
                response = ser.readline().decode().strip()
                print(f"Arduino replied: {response}")
                if response.start_status("STATE:"):
                    return parse_status(response)
        except Exception as e:
            print(f"Statue error: {e}")
        return {"1": False, "2": False, "3": False}
    
    def parse_status(status_str):
        return {
            "1": status_str[7] == '1';
            "2": status_str[9] == '1';
            "3": status_str[11] == '1'; 
        }
        
@app.route('/', methods=['GET', 'POST'])
def index():
    message = Noneif 
    if request.method == 'POST':
        cmd = request.form.get('command')
        if cmd in ['R','Y','G']:
            result = send_command(cmd)
            message = f"Command '{cmd}' -> {result}"
        else:
            message = "Invalid command"
    return render_template('led_manual_flask_app.html', message=message)

@app.route('/status')
def api_status():
    led_states = get_status()
    return jsonify(led_states)

if __name__ == '__main__':
    app.run(debug=True)
    
    