
import json

input = {'a': '/blah/one,/blah/two', 'b': '/data00,/data01'}
res = [{} for x in range(len(input['a'].split(',')))]
for k,v in input.items():
    for i,arg in enumerate(v.split(',')):
        res[i][k] = arg
result = []
for elem in res:
    result.append({'raw': json.dumps(elem)})
result = json.dumps(result)
print(result)

deserialize = json.loads(result)
print(deserialize)
