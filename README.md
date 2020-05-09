# Duktype

> Duktape for Typescript/Node.JS

[![npm version](https://badge.fury.io/js/%40caspertech%2Fduktype.svg)](https://badge.fury.io/js/%40caspertech%2Fduktype)
[![Build Status](https://travis-ci.org/CasperTech/duktype.svg?branch=master)](https://travis-ci.org/CasperTech/duktype)
[![Known Vulnerabilities](https://snyk.io/test/npm/@caspertech/duktype/badge.svg)](https://snyk.io/test/npm/@caspertech/duktype)
[![Dependencies](https://david-dm.org/CasperTech/duktype.svg)](https://david-dm.org/CasperTech/duktype.svg)

## About

Duktype (Duktape + Typescript) aims to be a complete node.js binding for the Duktape Javascript engine, complete with Typescript typings.

This allows you to run user scripts in a completely isolated environment.

This is a native C++ module, with no dependencies - it *should* compile on any platform.

## Security

Duktape and V8 don't share memory, so as long as Duktape itself remains secure ([exploits have existed](https://elakkod.se/blog/write-up-sec-t-2019-mirc2077/) in the past), it should not be possible for user scripts to escape their sandbox.

Note: The vulnerability flagged on the badge above is internal to node-cmake, the build system, and doesn't affect this package.

## Install

```bash
npm install --save @caspertech/duktype
```

## Usage

```typescript
import * as duktype from '@caspertech/duktype';

// Create a context which everything runs in
const ctx = new duktype.Context();

// Get the global object
const glob = ctx.getGlobalObject();

// Add a method to the global object
glob.setProperty('print', (any: any, number: any, of: any, args: any) =>
{
    console.log(any);
});

// Run some code which calls the method
ctx.eval(`print("I'm calling from Duktape!");`);

// Now maybe you want a console.log - let's add a console object
const obj = glob.createObject('console');

// Let's set another string property which is the tag to use for logging
obj.setProperty('tag', 'QUACK');

// Add a log method to console. You can pass parameters of almost any type, let's try a Date..
obj.setProperty('log', (date: Date, message: string) =>
{
    const tag = obj.getProperty('tag');
    console.log(date.toISOString() + ': [' + tag + '] ' + message);
});

// Let's run this thing
ctx.eval(`
print('Tag is: ' + console.tag);
console.log(new Date(), 'Console.log works!');
`);
```

## License

[MIT](http://vjpr.mit-license.org)
