
import binascii
from ctypes import *
import struct
from os import path

def init():
  global parser
  parser = cdll.LoadLibrary(path.dirname(path.realpath(__file__)) + "/../../core/slurp/libslurp.so")

  global READ_CALLBACK, REQUEST_CALLBACK, ERROR_CALLBACK
  READ_CALLBACK = CFUNCTYPE(c_int, POINTER(c_char), c_int)
  REQUEST_CALLBACK = CFUNCTYPE(None, POINTER(Request))
  ERROR_CALLBACK = CFUNCTYPE(None, POINTER(Error))

  parser.slurp_init_request_parser.argtypes = []
  parser.slurp_init_request_parser.restype = None
  parser.slurp_on_request.argtypes = [REQUEST_CALLBACK]
  parser.slurp_on_request.restype = None
  parser.slurp_parse_request.argtypes = [READ_CALLBACK, ERROR_CALLBACK]
  parser.slurp_parse_request.restype = None

  parser.slurp_init_request_parser()

def parse(stream, on_request, on_error):

  def read_callback(read_buffer_ptr, max_read):
    read_buffer_address = addressof(read_buffer_ptr.contents)
    read_buffer = (c_char * max_read).from_address(read_buffer_address)
    chunk = stream.read(max_read)
    read_buffer[:len(chunk)] = chunk
    return len(chunk)

  def request_callback(request_ptr):
    on_request(request_ptr.contents)

  def error_callback(error_ptr):
    on_error(error_ptr.contents)

  read_callback_ptr = READ_CALLBACK(read_callback)
  request_callback_ptr = REQUEST_CALLBACK(request_callback)
  error_callback_ptr = ERROR_CALLBACK(error_callback)

  parser.slurp_on_request(request_callback_ptr)
  parser.slurp_parse_request(read_callback_ptr, error_callback_ptr)

class Frame(object):

  DELIM = "\x7e"

  def __init__(self):
    pass

class Request(Frame, Structure):

  DELIM =","

  _fields_ = [
    ("program", c_char * 8),
    ("request", c_char * 8),
    ("arg_names", (c_char * 8) * 4),
    ("arg_values", (c_char * 4) * 4),
    ("args_length", c_int)
  ]

  def __init__(self, program, request, **args):
    self.program = program
    self.request = request
    for index, key in enumerate(args.keys()):
      if (index < 4):
        self.args_length += 1
      else:
        break
      self.arg_names[index] = create_string_buffer(key, 8)
      self.arg_values[index] = create_string_buffer(self.arg_to_bytes(args[key]), 4)

  def arg(self, name):
    arg_indexes = [index for index, key in enumerate(self.arg_names) if key.value == name]
    return self.arg_values[arg_indexes[0]].raw if arg_indexes else None

  def to_bytes(self):
    return [ord(char) for char in self.to_str()]

  def to_str(self):
    return "".join([Frame.DELIM, self.payload_to_str(), self.crc_to_str(), Frame.DELIM])

  def payload_to_str(self):
    return Request.DELIM.join([self.program, self.request, self.args_to_str()])

  def args_to_str(self):
    return Request.DELIM.join([
      Request.DELIM.join([self.arg_names[index].value, self.arg_values[index].raw])
      for index in range(self.args_length)
    ])

  def crc_to_str(self):
    return struct.pack('>l ', self.crc())

  def crc(self):
    return binascii.crc32(self.payload_to_str())

  def arg_to_bytes(self, value):
    return {
      int: lambda value: struct.pack('>l ', value)
    }.get(type(value), lambda value: "")(value)

  def send(self, stream):
    stream.write(self.to_str())

class Error(Structure):

  SLURP_ERROR_FRAMING = 1
  SLURP_ERROR_MISSING_PROGRAM = 2
  SLURP_ERROR_MISSING_REQUEST = 3
  SLURP_ERROR_MISSING_ARG_VALUE = 4

  _fields_ = [
    ("code", c_int)
  ]

  def __init__(self, code, **args):
    self.code = code
