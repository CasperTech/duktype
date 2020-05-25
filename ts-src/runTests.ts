import * as fs from 'fs';
import * as path from 'path';
const Mocha = require('mocha');

const mocha = new Mocha();

const testDir = path.resolve(__dirname + '/tests');

fs.readdirSync(testDir).filter(function (file)
{
    return file.substr(-3) === '.js';
}).forEach(function (file)
{
    mocha.addFile(
        path.join(testDir, file)
    );
});

mocha.run(function (failures: number)
{
    console.log('All done');
});