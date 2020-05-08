# Duktype

> Duktape for Typescript

[![npm version](https://badge.fury.io/js/%40caspertech%2Fduktype.svg)](https://badge.fury.io/js/%40caspertech%2Fduktype)
[![Build Status](https://travis-ci.org/CasperTech/duktype.svg?branch=master)](https://travis-ci.org/CasperTech/duktype)
[![Known Vulnerabilities](https://snyk.io/test/npm/@caspertech/duktype/badge.svg)](https://snyk.io/test/npm/@caspertech/duktype)
[![Dependencies](https://david-dm.org/CasperTech/duktype.svg)](https://david-dm.org/CasperTech/duktype.svg)

## About

Duktype (Duktape + Typescript) aims to be a complete node.js binding for the Duktape Javascript engine, complete with Typescript typings.

This allows you to run user scripts in a completely isolated environment.

This is a native C++ module - it *should* compile on any platform.

## Security

Duktape and V8 don't share memory, so as long as Duktape itself remains secure ([exploits]([https://elakkod.se/blog/write-up-sec-t-2019-mirc2077/]) have existed in the past), it should not be possible for user scripts to escape their sandbox.

## Install

```bash
npm install --save @caspertech/duktype
```

## Usage

For now, please see index.d.ts for the available interface. Full documentation will come when it's ready.

## License

[MIT](http://vjpr.mit-license.org)