# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: mediapipe/calculators/image/opencv_encoded_image_to_image_frame_calculator.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from mediapipe.framework import calculator_pb2 as mediapipe_dot_framework_dot_calculator__pb2
mediapipe_dot_framework_dot_calculator__options__pb2 = mediapipe_dot_framework_dot_calculator__pb2.mediapipe_dot_framework_dot_calculator__options__pb2


DESCRIPTOR = _descriptor.FileDescriptor(
  name='mediapipe/calculators/image/opencv_encoded_image_to_image_frame_calculator.proto',
  package='mediapipe',
  syntax='proto2',
  serialized_pb=_b('\nPmediapipe/calculators/image/opencv_encoded_image_to_image_frame_calculator.proto\x12\tmediapipe\x1a$mediapipe/framework/calculator.proto\"\xcd\x01\n/OpenCvEncodedImageToImageFrameCalculatorOptions\x12/\n apply_orientation_from_exif_data\x18\x01 \x01(\x08:\x05\x66\x61lse2i\n\x03\x65xt\x12\x1c.mediapipe.CalculatorOptions\x18\x8c\xfa\xd8\x90\x01 \x01(\x0b\x32:.mediapipe.OpenCvEncodedImageToImageFrameCalculatorOptions')
  ,
  dependencies=[mediapipe_dot_framework_dot_calculator__pb2.DESCRIPTOR,])
_sym_db.RegisterFileDescriptor(DESCRIPTOR)




_OPENCVENCODEDIMAGETOIMAGEFRAMECALCULATOROPTIONS = _descriptor.Descriptor(
  name='OpenCvEncodedImageToImageFrameCalculatorOptions',
  full_name='mediapipe.OpenCvEncodedImageToImageFrameCalculatorOptions',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='apply_orientation_from_exif_data', full_name='mediapipe.OpenCvEncodedImageToImageFrameCalculatorOptions.apply_orientation_from_exif_data', index=0,
      number=1, type=8, cpp_type=7, label=1,
      has_default_value=True, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
    _descriptor.FieldDescriptor(
      name='ext', full_name='mediapipe.OpenCvEncodedImageToImageFrameCalculatorOptions.ext', index=0,
      number=303447308, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=True, extension_scope=None,
      options=None),
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=134,
  serialized_end=339,
)

DESCRIPTOR.message_types_by_name['OpenCvEncodedImageToImageFrameCalculatorOptions'] = _OPENCVENCODEDIMAGETOIMAGEFRAMECALCULATOROPTIONS

OpenCvEncodedImageToImageFrameCalculatorOptions = _reflection.GeneratedProtocolMessageType('OpenCvEncodedImageToImageFrameCalculatorOptions', (_message.Message,), dict(
  DESCRIPTOR = _OPENCVENCODEDIMAGETOIMAGEFRAMECALCULATOROPTIONS,
  __module__ = 'mediapipe.calculators.image.opencv_encoded_image_to_image_frame_calculator_pb2'
  # @@protoc_insertion_point(class_scope:mediapipe.OpenCvEncodedImageToImageFrameCalculatorOptions)
  ))
_sym_db.RegisterMessage(OpenCvEncodedImageToImageFrameCalculatorOptions)

_OPENCVENCODEDIMAGETOIMAGEFRAMECALCULATOROPTIONS.extensions_by_name['ext'].message_type = _OPENCVENCODEDIMAGETOIMAGEFRAMECALCULATOROPTIONS
mediapipe_dot_framework_dot_calculator__options__pb2.CalculatorOptions.RegisterExtension(_OPENCVENCODEDIMAGETOIMAGEFRAMECALCULATOROPTIONS.extensions_by_name['ext'])

# @@protoc_insertion_point(module_scope)
