
import StringIO
import struct

from slurp import request

basic_request = request.Request("chaser", "start")
basic_request_str = "~chaser,starts~"

geo_request = request.Request("geo", "map", lat = 120, lon = -30, zoom = 3)
geo_request_str = "~geo,map,lat,\x00\x00\x00x,lon,\xff\xff\xff\xe2,zoom,\x00\x00\x00\x03\xa7\x9b\x91\x7f~"
geo_request_hex = "7e67656f2c6d61702c6c61742c000000782c6c6f6e2cffffffe22c7a6f6f6d2c00000003a79b917f7e"

def test_can_be_serialized_to_a_stream():
	stream = StringIO.StringIO()
	geo_request.send(stream)
	assert stream.getvalue().encode('hex') == geo_request_hex
	stream.close()

def test_can_be_parsed_from_a_stream():
	def request_callback(request):
		assert request.program == basic_request.program
		assert request.request == basic_request.request

	stream = StringIO.StringIO(basic_request_str);
	request.parse(stream, request_callback)
