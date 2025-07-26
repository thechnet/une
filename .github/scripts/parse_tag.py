import sys

EMPTY_TAG_VALUE = 'null'

version = sys.argv[1][1::]
[_, build] = version.split('+', 1) if '+' in version else [version, '']
[iteration, release] = _.split('-', 1) if '-' in _ else [_, '']

release_tags: dict[str, str] = {}
for tag in release.split('-') if release else '':
    [key, value] = tag.split('.') if '.' in tag else [tag, EMPTY_TAG_VALUE]
    release_tags[key] = value

build_tags: dict[str, str] = {}
for tag in build.split('-') if build else '':
    [key, value] = tag.split('.') if '.' in tag else [tag, EMPTY_TAG_VALUE]
    build_tags[key] = value

print(f'iteration={iteration}')

for tag in release_tags:
    print(f'release-{tag}={release_tags[tag]}')

for tag in build_tags:
    print(f'build-{tag}={build_tags[tag]}')
