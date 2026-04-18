from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import struct

app = FastAPI()

# Allow CORS so your React app (running on e.g. localhost:3000) can call the API
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:3000"],  # adjust to your React dev server URL
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/fft/")
def get_fft():
    # Convert 0xAAAAAAAA to float32 using struct
    hex_val = 0xAAAAAAAA
    float_val = struct.unpack('f', struct.pack('I', hex_val))[0]
    # Create list of 256 repeated values
    fft_data = [float_val] * 256
    
    # Return JSON: list of floats
    return {"fft": fft_data}
