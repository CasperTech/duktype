const duktape = require('node-cmake')('duktype');

duktape.Context.prototype.enableTimers = function()
{
    if (this.timersEnabled)
    {
        return;
    }

    this.pendingTimers = {};
    this.lastTimerID = 0;

    const scope = this.getGlobalObject();
    scope.setProperty('setTimeout', (cb, timeout) =>
    {
        const ourID = this.lastTimerID++;
        this.pendingTimers[ourID] = {
            type: 'timeout',
            timeout: setTimeout(() =>
            {
                delete this.pendingTimers[ourID];
                cb.call();
            }, parseInt(timeout, 10))
        };
        return ourID;
    });

    scope.setProperty('setInterval', (func, timeout) =>
    {
        const ourID = this.lastTimerID++;
        this.pendingTimers[ourID] = {
            type: 'interval',
            timeout: setInterval(() =>
            {
                func.call();
            }, parseInt(timeout, 10))
        };
        return ourID;
    });

    scope.setProperty('clearTimeout', (ref) =>
    {
        const refSan = parseInt(ref, 10);
        if (this.pendingTimers[refSan] !== undefined && this.pendingTimers[refSan].type === 'timeout')
        {
            clearTimeout(this.pendingTimers[refSan].timeout);
            delete this.pendingTimers[refSan];
        }
    });

    scope.setProperty('clearInterval', (ref) =>
    {
        const refSan = parseInt(ref, 10);
        if (this.pendingTimers[refSan] !== undefined && this.pendingTimers[refSan].type === 'interval')
        {
            clearInterval(this.pendingTimers[refSan].timeout);
            delete this.pendingTimers[refSan];
        }
    });

    this.timersEnabled = true;
};

duktape.Context.prototype.enablePromises = function()
{
    if (this.promisesEnabled)
    {
        return;
    }

    if (!this.timersEnabled)
    {
        this.enableTimers();
    }

    this.eval(`!function(e){"object"==typeof exports&&"undefined"!=typeof module||"function"!=typeof define||!define.amd?e():define(e)}(function(){"use strict";function e(e){var n=this.constructor;return this.then(function(t){return n.resolve(e()).then(function(){return t})},function(t){return n.resolve(e()).then(function(){return n.reject(t)})})}var n=setTimeout;function t(e){return e&&void 0!==e.length}function o(){}function r(e){if(!(this instanceof r))throw new TypeError("Promises must be constructed via new");if("function"!=typeof e)throw new TypeError("not a function");this._state=0,this._handled=!1,this._value=void 0,this._deferreds=[],a(e,this)}function i(e,n){for(;3===e._state;)e=e._value;0!==e._state?(e._handled=!0,r._immediateFn(function(){var t=1===e._state?n.onFulfilled:n.onRejected;if(null!==t){var o;try{o=t(e._value)}catch(s){return void u(n.promise,s)}f(n.promise,o)}else(1===e._state?f:u)(n.promise,e._value)})):e._deferreds.push(n)}function f(e,n){try{if(n===e)throw new TypeError("A promise cannot be resolved with itself.");if(n&&("object"==typeof n||"function"==typeof n)){var t=n.then;if(n instanceof r)return e._state=3,e._value=n,void c(e);if("function"==typeof t)return void a(function(e,n){return function(){e.apply(n,arguments)}}(t,n),e)}e._state=1,e._value=n,c(e)}catch(i){u(e,i)}}function u(e,n){e._state=2,e._value=n,c(e)}function c(e){2===e._state&&0===e._deferreds.length&&r._immediateFn(function(){e._handled||r._unhandledRejectionFn(e._value)});for(var n=0,t=e._deferreds.length;n<t;n++)i(e,e._deferreds[n]);e._deferreds=null}function a(e,n){var t=!1;try{e(function(e){t||(t=!0,f(n,e))},function(e){t||(t=!0,u(n,e))})}catch(o){if(t)return;t=!0,u(n,o)}}r.prototype.catch=function(e){return this.then(null,e)},r.prototype.then=function(e,n){var t=new this.constructor(o);return i(this,new function(e,n,t){this.onFulfilled="function"==typeof e?e:null,this.onRejected="function"==typeof n?n:null,this.promise=t}(e,n,t)),t},r.prototype.finally=e,r.all=function(e){return new r(function(n,r){if(!t(e))return r(new TypeError("Promise.all accepts an array"));var i=Array.prototype.slice.call(e);if(0===i.length)return n([]);var f=i.length;function u(e,t){try{if(t&&("object"==typeof t||"function"==typeof t)){var c=t.then;if("function"==typeof c)return void c.call(t,function(n){u(e,n)},r)}i[e]=t,0==--f&&n(i)}catch(o){r(o)}}for(var c=0;c<i.length;c++)u(c,i[c])})},r.resolve=function(e){return e&&"object"==typeof e&&e.constructor===r?e:new r(function(n){n(e)})},r.reject=function(e){return new r(function(n,t){t(e)})},r.race=function(e){return new r(function(n,o){if(!t(e))return o(new TypeError("Promise.race accepts an array"));for(var i=0,f=e.length;i<f;i++)r.resolve(e[i]).then(n,o)})},r._immediateFn="function"==typeof setImmediate?function(e){setImmediate(e)}:function(e){n(e,0)},r._unhandledRejectionFn=function(e){void 0!==console&&console&&console.warn("Possible Unhandled Promise Rejection:",e)};var s=new Function("return this;")();"function"!=typeof s.Promise?s.Promise=r:s.Promise.prototype.finally||(s.Promise.prototype.finally=e)});`);

    this.promisesEnabled = true;
};

module.exports = duktape;