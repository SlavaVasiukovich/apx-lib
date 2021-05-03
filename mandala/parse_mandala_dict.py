#!/usr/bin/env python
#
# APX Autopilot project <http://docs.uavos.com>
#
# Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
# All rights reserved
#
# This file is part of APX Shared Libraries.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

import argparse
import sys
import os

import simplejson
import yaml


# construct dict based on file [config] or data
class Mandala(dict):
    # static members
    configExt = '.yml'
    config = None
    inheritance = list()

    def __init__(self, root_config=None, data=None, parent=None):
        dict.__init__(self)

        if root_config:
            assert(data is None and parent is None)
            data = self.read_config(root_config)
            Mandala.config = root_config
            Mandala.inheritance = data['inheritance']

        # inherit parent props
        if parent:
            for key in parent:
                if key in Mandala.inheritance:
                    self[key] = parent[key]
            if not 'title' in data and 'suffix' in data:
                data['title'] = (parent['title'] + ' ' +
                                 data['suffix']).strip()

        for key in data:
            if key != 'content':
                self[key] = data[key]

        if 'content' in data:
            content = data['content']
            self['content'] = list()
            if type(content) is not list:
                conf = os.path.join(os.path.dirname(
                    Mandala.config), content + Mandala.configExt)
                content = self.read_config(conf)
            for i in list(content):
                # if not 'name' in i:
                #     continue
                self['content'].append(Mandala(data=i, parent=self).copy())

    def read_config(self, config):
        # print('Reading {}...'.format(config))
        with open(config, 'r') as f:
            return yaml.load(f.read(), Loader=yaml.Loader)


def main():
    parser = argparse.ArgumentParser(
        description='Parse Mandala dict YAML files and print JSON')
    parser.add_argument('--dict', required=False,
                        help='path to dict YAML file')
    parser.add_argument('--out', required=False,
                        help='path to output JSON file')
    args = parser.parse_args()

    args.dict = os.path.join(os.path.dirname(
        os.path.abspath(__file__)), 'dict/mandala.yml')

    mandala = Mandala(root_config=args.dict)

    if args.out:
        with open(args.out, 'w') as f:
            simplejson.dump(mandala, f, sort_keys=True, indent=2)
            f.write('\n')
    else:
        print(simplejson.dumps(mandala))


if __name__ == "__main__":
    sys.exit(main())
