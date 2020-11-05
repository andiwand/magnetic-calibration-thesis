#!/usr/bin/env python

from indoors.pipeline.protocol import event_pb2
from google.protobuf.internal.decoder import _DecodeVarint32
import pandas as pd

def load(path):
    result = {
        'begin_time': None,
        'end_time': None,
        'duration': None,
        'channels': {},
    }

    with open(path, 'rb') as f:
        first = True
        while True:
            len_data = f.read(4)
            if not len_data:
                break
            msg_len, len_len = _DecodeVarint32(len_data, 0)
            f.seek(len_len - 4, 1)

            msg = event_pb2.Event()
            msg.ParseFromString(f.read(msg_len))

            if first:
                assert(msg.HasField('hello'))

                result['begin_time'] = msg.t

                for channel in msg.hello.channels:
                    result['channels'][channel.channel] = {
                        'annotation': channel.annotation,
                        'data': [],
                    }

                first = False
                continue

            assert(not msg.HasField('hello'))
            if msg.HasField('bye'):
                break
            event = {'t': msg.t}
            if msg.HasField('clock'):
                event['unixUtc'] = msg.clock.unix_utc
            elif msg.HasField('vector2'):
                event['x'] = msg.vector2.x
                event['y'] = msg.vector2.y
            elif msg.HasField('vector3'):
                event['x'] = msg.vector3.x
                event['y'] = msg.vector3.y
                event['z'] = msg.vector3.z
            elif msg.HasField('quaternion'):
                event['w'] = msg.quaternion.w
                event['x'] = msg.quaternion.x
                event['y'] = msg.quaternion.y
                event['z'] = msg.quaternion.z
            result['channels'][msg.channel]['data'].append(event)

    for k, v in result['channels'].items():
        v['data'] = pd.DataFrame(v['data'])

    return result
