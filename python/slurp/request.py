
import binascii
from ctypes import *
import struct
from os import path

def init():
  global parser
  parser = cdll.LoadLibrary(path.dirname(path.realpath(__file__)) + "/../../core/slurp/libslurp.so")

  global READ_CALLBACK, REQUEST_CALLBACK
  READ_CALLBACK = CFUNCTYPE(c_int, POINTER(c_char), c_int)
  REQUEST_CALLBACK = CFUNCTYPE(None, POINTER(REQUEST))

  parser.slurp_init_request_parser.argtypes = []
  parser.slurp_init_request_parser.restype = None
  parser.slurp_on_request.argtypes = [REQUEST_CALLBACK]
  parser.slurp_on_request.restype = None
  parser.slurp_parse_request.argtypes = [READ_CALLBACK]
  parser.slurp_parse_request.restype = c_int

  parser.slurp_init_request_parser()

def parse(stream, on_request):

  def read_callback(read_buffer_ptr, max_read):
    read_buffer_address = addressof(read_buffer_ptr.contents)
    read_buffer = (c_char * max_read).from_address(read_buffer_address)
    chunk = stream.read(max_read)
    read_buffer[:len(chunk)] = chunk
    return len(chunk)

  def request_callback(request_ptr):
    on_request(request_ptr.contents)

  read_callback_ptr = READ_CALLBACK(read_callback)
  request_callback_ptr = REQUEST_CALLBACK(request_callback)

  parser.slurp_on_request(request_callback_ptr)
  parser.slurp_parse_request(read_callback_ptr)

class Frame(object):

  DELIM = "\x7e"

  def __init__(self):
    pass

class Request(Frame):

  DELIM =","

  def __init__(self, program, request, **kwargs):
    self.program = program
    self.request = request
    self.kwargs = kwargs

  def to_bytes(self):
    return [ord(char) for char in self.to_str()]

  def to_str(self):
    return "".join([Frame.DELIM, self.payload_to_str(), self.crc_to_str(), Frame.DELIM])

  def payload_to_str(self):
    return Request.DELIM.join([self.program, self.request, self.kwargs_to_str()])

  def kwargs_to_str(self):
    return Request.DELIM.join([
      Request.DELIM.join([key, self.arg_to_bytes(value)])
      for key, value in self.kwargs.iteritems()
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

class REQUEST(Structure):
  _fields_ = [
    ("program", c_char * 8),
    ("request", c_char * 8)
  ]
