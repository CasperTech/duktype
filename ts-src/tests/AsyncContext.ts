import 'mocha';
import { AsyncContext, AsyncObjectScope } from '../../index';
import * as uuid from 'uuid'

let ctx: AsyncContext | undefined;
let globalObj: AsyncObjectScope | undefined;
let dumpResponse = '';

describe('AsyncContext', () =>
{
    ctx = new AsyncContext();

    it('should instantiate properly', async() =>
    {
        if (ctx === undefined || !(ctx instanceof AsyncContext))
        {
            throw new Error();
        }
    });

    it('should evaluate a simple numeric expression', async () =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        const result = await ctx.eval('4*4/2');
        if (result !== 8)
        {
            throw new Error();
        }
    });

    it('should give a valid global object', async () =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        globalObj = await ctx.getGlobalObject();
        const result = JSON.parse(await ctx.eval('JSON.stringify(this)'));
        if (result === undefined || result.performance === undefined)
        {
            throw new Error();
        }
    });

    it('should allow a method to be created on the global object', async () =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.setProperty('dump', (str: string) =>
        {
            dumpResponse = str;
        });
    });

    it('should allow the method to be called by the context', async () =>
    {
        const testID = uuid.v4();
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        await ctx.eval('dump(\'' + testID + '\');')
        if (dumpResponse !== testID)
        {
            throw new Error('Dumped ID ' + dumpResponse + ' does not match expected value ' + testID);
        }
    });
});

describe('Global Object', () =>
{
    it('should allow the method to be called directly on the global object', async () =>
    {
        dumpResponse = 'INVALID';
        const testID = uuid.v4();
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.callMethod('dump', testID)
        if (dumpResponse !== testID)
        {
            throw new Error('Got ' + dumpResponse + ' instead of expected ' + testID);
        }
    });

    it('should allow a property to be set', async () =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.setProperty('test', 'testValue');
        const obj = JSON.parse(await ctx.eval('JSON.stringify(this)'));
        if (obj.test !== 'testValue')
        {
            throw new Error('testValue not found');
        }
    });

    it('should allow a property value to be retrieved', async () =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        if (await globalObj.getProperty('test') !== 'testValue')
        {
            throw(new Error());
        }
    });

    it('should allow a property value to be deleted', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.deleteProperty('test');
    });

    it('should return undefined when getting a deleted property', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        if (await globalObj.getProperty('test') !== undefined)
        {
            throw new Error();
        }
    });

    const testFunctionRet = uuid.v4();
    it('should allow a function property to be set', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.setProperty('test', (seed: string) =>
        {
            return testFunctionRet + seed;
        });
    });

    it('should allow a function property to be called', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.callMethod('test');
    });

    it('should get the correct return value from the function property', async () =>
    {
        const seed = uuid.v4();
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        if (await globalObj.callMethod('test', seed) !== testFunctionRet + seed)
        {
            throw new Error();
        }
    });
});

let childObject: AsyncObjectScope | undefined;

describe('ObjectScope', () =>
{
    it('should allow a child object to be created ', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        childObject = await globalObj.createObject('foo');
        if (childObject === undefined)
        {
            throw new Error('Returned object is undefined');
        }
    });

    it('should contain the newly created child object', async() =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        const result = JSON.parse(await ctx.eval('JSON.stringify(this)'));
        if (result.foo === undefined)
        {
            throw new Error();
        }
    });

    let furtherChild: AsyncObjectScope | undefined;
    it('should allow a further child to be created on that child', async() =>
    {
        if (childObject === undefined)
        {
            throw new Error('childObject is undefined');
        }

        try
        {
            furtherChild = await childObject.createObject('bar');
        }
        catch(e)
        {
            console.error(e);
            throw(e);
        }
    });

    it('should not show the sub-child on the global object', async () =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        const result = JSON.parse(await ctx.eval('JSON.stringify(this)'));
        if (result.bar !== undefined)
        {
            throw new Error();
        }
    });

    it('should show the sub-child on the first child object', async() =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        const result = JSON.parse(await ctx.eval('JSON.stringify(this.foo)'));
        if (result.bar === undefined)
        {
            throw new Error()
        }
    });

    it('should be able to access a method on the global object from the second child', async () =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.setProperty('howmany', () => {
            return 70;
        })
        await ctx.eval(`
                foo.bar.count = function(start) {
                    return start + howmany();
                };
            `);
        if (furtherChild !== undefined)
        {
            const result = await furtherChild.callMethod('count', 12);
            if (result !== 82)
            {
                throw new Error();
            }
        }
    });

    it('should be able to pass functions with any number of arguments', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.setProperty('multipleArgumentFunc', (...args: any[]) => {
            return args.length;
        })

        const result = await globalObj.callMethod('multipleArgumentFunc', 'hello', 42, new Date(), Buffer.alloc(4), {hello: 'test'}, new Error('the cake is a lie'));
        if (result !== 6)
        {
            throw new Error(result + ' arguments received')
        }
    });

    it('should still be able to access the deleted object due to reference', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.deleteProperty('foo');
        if (furtherChild !== undefined)
        {
            const result = await furtherChild.callMethod('count', 12);
            if (result !== 82)
            {
                throw new Error();
            }
        }
    });

    it('objects should be garbage collected properly', async() =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        childObject = undefined;
        furtherChild = undefined;
        global.gc();
        await ctx.runGC();
        const objectRefs = ctx.getObjectReferenceCount();
        if (objectRefs !== 1)
        {
            throw new Error('Object ref count is ' + objectRefs)
        }
    });
});

describe('Buffers', () =>
{
    it('should pass a Node.JS buffer object with data intact and back again', async() =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        await ctx.eval(`
        function checkValue(val)
        {
            return {
                value: val,
                stringified: JSON.stringify(val),
                type: typeof val,
                construct: (typeof val === 'object' && val !== null) ? val.constructor.name : null
            }
        }
        `);


        const buf = Buffer.from([1,2,3,4,5]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            console.error('Value is undefined');
            return new Error('Value is undefined');
        }
        if (typeof result.value !== 'object')
        {
            console.error('Value is not an object');
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            console.error('Type is not an object');
            throw new Error('Type is not an object');
        }
        if (!(result.value instanceof Buffer) || result.construct !== 'Buffer')
        {
            console.error('Returned value is not a buffer (its a ' + result.value.constructor.name + ')');
            throw new Error('Returned value is not a buffer (its a ' + result.value.constructor.name + ')');
        }
        if (Buffer.compare(buf, result.value) !== 0)
        {
            console.error('Buffers are not the same (compare)');
            throw new Error('Buffers are not the same (compare)');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed.data[x])
            {
                console.error('Buffers are not the same (manual)');
                throw new Error('Buffers are not the same (manual)');
            }
        }
    });
});
describe('Primitive Types', () =>
{
    describe('Boolean', () =>
    {
        it('should pass with value and type intact and back again', async () =>
        {
            let val = true;
            if (globalObj === undefined)
            {
                throw new Error('GlobalObject is undefined');
            }
            let result = await globalObj.callMethod('checkValue', val);
            if (result.value === undefined)
            {
                throw new Error('Value is undefined');
            }
            if (typeof result.value !== 'boolean')
            {
                throw new Error('Value is not a boolean');
            }
            if (result.type !== 'boolean')
            {
                throw new Error('Type is not a boolean');
            }
            if (result.value !== val)
            {
                throw new Error('Returned value differs from transmitted value');
            }

            val = false;
            if (globalObj === undefined)
            {
                throw new Error('GlobalObject is undefined');
            }
            result = await globalObj.callMethod('checkValue', val);
            if (result.value === undefined)
            {
                throw new Error('Value is undefined');
            }
            if (typeof result.value !== 'boolean')
            {
                throw new Error('Value is not a boolean');
            }
            if (result.type !== 'boolean')
            {
                throw new Error('Type is not a boolean');
            }
            if (result.value !== val)
            {
                throw new Error('Returned value differs from transmitted value');
            }
        });
    });
    describe('Number', () =>
    {
        it('should pass with value and type intact and back again', async() =>
        {
            let val = 0.192809;
            if (globalObj === undefined)
            {
                throw new Error('GlobalObject is undefined');
            }
            let result = await globalObj.callMethod('checkValue', val);
            if (result.value === undefined)
            {
                throw new Error('Value is undefined');
            }
            if (typeof result.value !== 'number')
            {
                throw new Error('Value is not a number');
            }
            if (result.type !== 'number')
            {
                throw new Error('Type is not a number');
            }
            if (result.value !== val)
            {
                throw new Error('Returned value differs from transmitted value');
            }
        });
    });
    describe('UTF-8 String', () =>
    {
        it('should pass with value and type intact and back again', async() =>
        {
            let val = `An preost wes on leoden, Laȝamon was ihoten. He wes Leovenaðes sone -- 
                       liðe him be Drihten. He wonede at Ernleȝe at æðelen are chirechen, 
                       Uppen Sevarne staþe, sel þar him þuhte, Onfest Radestone, þer he bock radde.`;
            if (globalObj === undefined)
            {
                throw new Error('GlobalObject is undefined');
            }

            let result = await globalObj.callMethod('checkValue', val);
            if (result.value === undefined)
            {
                throw new Error('Value is undefined');
            }
            if (typeof result.value !== 'string')
            {
                throw new Error('Value is not a string');
            }
            if (result.type !== 'string')
            {
                throw new Error('Type is not a string');
            }
            if (result.value !== val)
            {
                throw new Error('Returned value differs from transmitted value');
            }
        });
    });
    describe('Undefined', () =>
    {
        it('should pass with type intact and back again', async() =>
        {
            let val = undefined;
            if (globalObj === undefined)
            {
                throw new Error('GlobalObject is undefined');
            }
            let result = await globalObj.callMethod('checkValue', val);
            if (typeof result.value !== 'undefined')
            {
                throw new Error('Value is not undefined');
            }
            if (result.type !== 'undefined')
            {
                throw new Error('Type is not undefined');
            }
            if (result.value !== val)
            {
                throw new Error('Returned value differs from transmitted value');
            }
        });
    });
    describe('Null', () =>
    {
        it('should pass with type intact and back again', async() =>
        {
            let val = null;
            if (globalObj === undefined)
            {
                throw new Error('GlobalObject is undefined');
            }
            let result = await globalObj.callMethod('checkValue', val);
            if (typeof result.value !== 'object')
            {
                throw new Error('Value is not object');
            }
            if (result.type !== 'object')
            {
                throw new Error('Type is not object');
            }
            if (result.value !== val)
            {
                throw new Error('Returned value differs from transmitted value');
            }
        });
    });
});
describe('Arrays', () =>
{
    it('should pass an array with data intact and back again', async () =>
    {
        const buf = [0, 291, -220, 0.329192, 'hello', ['anotherarray', {key: 'value'}], {key: 'value'}, true, false];
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an array');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        if (!(result.value instanceof Array) || result.construct !== 'Array')
        {
            throw new Error('Returned value is not an Array (its a ' + result.value.constructor.name + ')');
        }
        if (JSON.stringify(buf) !== JSON.stringify(result.value))
        {
            throw new Error('Returned value differs from duktape stringified value');
        }
        if (JSON.stringify(buf) !== JSON.stringify(JSON.parse(result.stringified)))
        {
            throw new Error('Node stringified value differs from duktape re-stringified value');
        }
    });
});
describe('TypedArrays', () =>
{
    it('should pass a Uint8ClampedArray with data intact and back again', async() =>
    {
        const buf = Uint8ClampedArray.from([1,2,3,4,5]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        if (!(result.value instanceof Uint8ClampedArray) || result.construct !== 'Uint8ClampedArray')
        {
            throw new Error('Returned value is not a Uint8ClampedArray (its a ' + result.value.constructor.name + ')');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
            {
                throw new Error('Arrays do not match');
            }
        }
    });

    it('should pass a Uint8Array with data intact and back again', async() =>
    {
        const buf = Uint8Array.from([1,2,3,4,5]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        if (!(result.value instanceof Uint8Array)  || result.construct !== 'Uint8Array')
        {
            throw new Error('Returned value is not a UInt8Array (its a ' + result.value.constructor.name + ')');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
            {
                throw new Error('Arrays do not match');
            }
        }
    });

    it('should pass a Uint16Array with data intact and back again', async() =>
    {
        const buf = Uint16Array.from([1,2,3,4,5]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        if (!(result.value instanceof Uint16Array)  || result.construct !== 'Uint16Array')
        {
            throw new Error('Returned value is not a Uint16Array (its a ' + result.value.constructor.name + ')');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
            {
                throw new Error('Arrays do not match');
            }
        }
    });

    it('should pass a Uint32Array with data intact and back again', async() =>
    {
        const buf = Uint32Array.from([1,2,3,4,5]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        if (!(result.value instanceof Uint32Array)  || result.construct !== 'Uint32Array')
        {
            throw new Error('Returned value is not a Uint32Array (its a ' + result.value.constructor.name + ')');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
            {
                throw new Error('Arrays do not match');
            }
        }
    });

    it('should pass a Int8Array with data intact and back again', async() =>
    {
        const buf = Int8Array.from([-1,2,-3,4,-5]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (!(result.value instanceof Int8Array)  || result.construct !== 'Int8Array')
        {
            throw new Error('Returned value is not an Int8Array (its a ' + result.value.constructor.name + ')');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
            {
                throw new Error('Arrays do not match');
            }
        }
    });

    it('should pass a Int16Array with data intact and back again', async() =>
    {
        const buf = Int16Array.from([-1,2,-3,4,-5]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (!(result.value instanceof Int16Array)  || result.construct !== 'Int16Array')
        {
            throw new Error('Returned value is not an Int16Array (its a ' + result.value.constructor.name + ')');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
            {
                throw new Error('Arrays do not match');
            }
        }
    });

    it('should pass a Int32Array with data intact and back again', async() =>
    {
        const buf = Int32Array.from([-1,2,-3,4,-5]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (!(result.value instanceof Int32Array) || result.construct !== 'Int32Array')
        {
            throw new Error('Returned value is not an Int32Array (its a ' + result.value.constructor.name + ')');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
            {
                throw new Error('Arrays do not match');
            }
        }
    });

    it('should pass a Float32Array with data intact and back again', async() =>
    {
        const buf = Float32Array.from([-1192.19,222.212,-3.1091,429192.021,3.14159265358979323846264338327950288419716939937510]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (!(result.value instanceof Float32Array) || result.construct !== 'Float32Array')
        {
            throw new Error('Returned value is not a Float32Array (its a ' + result.value.constructor.name + ')');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
            {
                throw new Error('Arrays do not match');
            }
        }
    });

    it('should pass a Float64Array with data intact and back again', async() =>
    {
        const buf = Float64Array.from([-1192.19,222.212,-3.1091,429192.021,3.14159265358979323846264338327950288419716939937510]);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (!(result.value instanceof Float64Array) || result.construct !== 'Float64Array')
        {
            throw new Error('Returned value is not a Float64Array (its a ' + result.value.constructor.name + ')');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        const parsed = JSON.parse(result.stringified);
        for (let x = 0; x < buf.length; x++)
        {
            if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
            {
                throw new Error('Arrays do not match');
            }
        }
    });

    it('should pass an ArrayBuffer with data intact and back again', async() =>
    {
        const buf = new ArrayBuffer(8);
        const dView = new DataView(buf);
        // @ts-ignore
        dView.setBigInt64(0, 9121312131814171817n, true);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (!(result.value instanceof ArrayBuffer) || result.construct !== 'ArrayBuffer')
        {
            throw new Error('Returned value is not an ArrayBuffer (its a ' + result.value.constructor.name + ')');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        const secondDView = new DataView(result.value);
        // @ts-ignore
        if (secondDView.getBigInt64(0, true) !== 9121312131814171817n)
        {
            throw new Error('Arrays do not match');
        }
    });

    it('should pass a DataView with data intact and back again', async() =>
    {
        const b = new ArrayBuffer(8);
        const buf = new DataView(b);
        // @ts-ignore
        buf.setBigInt64(0, 9121312131814171817n, true);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', buf);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (!(result.value instanceof DataView) || result.construct !== 'DataView')
        {
            throw new Error('Returned value is not a DataView (its a ' + result.value.constructor.name + ')');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        // @ts-ignore
        if (result.value.getBigInt64(0, true) !== 9121312131814171817n)
        {
            throw new Error('Arrays do not match');
        }
    });
});
describe('Dates', () =>
{
    it('should pass a Date object with data intact and back again', async() =>
    {
        const val = new Date(new Date().getTime() - 2764800000);
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', val);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        if (!(result.value instanceof Date) || result.construct !== 'Date')
        {
            throw new Error('Returned value is not a date (its a ' + result.value.constructor.name + ')');
        }
        if (val.getTime() !== result.value.getTime())
        {
            throw new Error('Dates are not the same (getTime)');
        }
        if (val.toISOString() !== result.value.toISOString())
        {
            throw new Error('Dates are not the same (ISOString)');
        }
    });
});
describe('Errors', () =>
{
    it('should pass an Error object with message intact and back again', async() =>
    {
        const val = new Error(uuid.v4());
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        const result = await globalObj.callMethod('checkValue', val);
        if (result.value === undefined)
        {
            throw new Error('Value is undefined');
        }
        if (typeof result.value !== 'object')
        {
            throw new Error('Value is not an object');
        }
        if (result.type !== 'object')
        {
            throw new Error('Type is not an object');
        }
        if (!(result.value instanceof Error) || result.construct !== 'Error')
        {
            throw new Error('Returned value is not an Error (its a ' + result.value.constructor.name + ')');
        }
        if ('Uncaught Error: ' + val.message !== result.value.message)
        {
            throw new Error('Messages are not the same (' + val.message + ' vs ' + result.value.message + ')');
        }
    });

    it('should be possible to catch a thrown exception', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.setProperty('doABadThing', () => {
            throw new Error('A bad thing happened');
        });

        try
        {
            await globalObj.callMethod('doABadThing');
            return new Error('An error was not thrown');
        }
        catch(error)
        {
            if (error.message !== 'Error: Uncaught Error: A bad thing happened')
            {
                throw new Error('Unexpected error message caught');
            }
        }
    });

    it('should be possible for duktape to catch an exception thrown in node', async () =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        const result = await ctx.eval(`(function () {
            try { doABadThing(); } catch (error) { return error.message }
        })()`)
        if (result !== 'Uncaught Error: A bad thing happened')
        {
            throw new Error()
        }
    });
});
describe('Promises and Timers', () =>
{
    it('should enable promises and timers successfully', async() =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        await ctx.enablePromises();
    });
    it('should delay eval by at least 100ms', async () =>
    {
        const d = new Date().getTime();
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        const p = ctx.eval(`
            new Promise(function (resolve, reject) {
                setTimeout(function(){
                    resolve();                
                }, 100);
            });
        `);
        const result = await p;
        const time = (new Date().getTime() - d);
        if (time < 100)
        {
            throw new Error('Time in promise less than 100ms: ' + time);
        }
    });
    it('should pass promises in both directions', async () =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.setProperty('promiseTest', async() => {
            return 'promiseWorked'
        });
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        const prom1 = await ctx.eval(`new Promise(function(resolve, reject){ promiseTest().then(function (result){ resolve(result);  }) });`);
        const result = await prom1;
        if (result === 'promiseWorked')
        {
            return;
        }
        else
        {
            return new Error();
        }
    });
});
describe('Garbage Collection', async() =>
{
    it('should run successfully', async() =>
    {
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        await ctx.runGC();
    });
});

describe('Complex operations', async() =>
{
    it('should stack calls without issue', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        await globalObj.setProperty('addOne', (num: number) => {
            return num + 1;
        })
        await globalObj.setProperty('makeEight', async () => {
            if (globalObj === undefined)
            {
                throw new Error('GlobalObject is undefined');
            }
            return await globalObj.callMethod('addOne', await globalObj.callMethod('addOne', await globalObj.callMethod('addOne', await globalObj.callMethod('addOne', 4))));
        });
        const result = await globalObj.callMethod('makeEight');
        if (result !== 8)
        {
            throw new Error('Unexpected return value: ' + result);
        }
    });


    it('should handle many simultaneous calls', async() =>
    {
        const promises: Promise<number>[] = [];
        if (globalObj === undefined)
        {
            throw new Error('GlobalObject is undefined');
        }
        for (let x = 0; x < 100; x++)
        {
            promises.push(globalObj.callMethod('makeEight'));
        }
        for (const promise of promises)
        {
            const result = await (await promise);
            if (result !== 8)
            {
                throw new Error('Invalid result received');
            }
        }
    });

    it('should handle many simultaneous eval calls', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('Context is undefined');
        }

        await globalObj.setProperty('test1', () => {
            return 'bacon';
        });
        await globalObj.setProperty('test2', () => {
            return ' and ';
        });
        await globalObj.setProperty('test3', () => {
            return 'chips';
        });


        async function run()
        {
            if (ctx === undefined)
            {
                throw new Error('Context is undefined');
            }
            return await ctx.eval('test1() + test2() + test3();');
        }
        const promises: Promise<string>[] = [];
        for (let x = 0; x < 100; x++)
        {
            promises.push(run());
        }

        for (const promise of promises)
        {
            const result = await promise;
            if (result !== 'bacon and chips')
            {
                throw new Error('Invalid result received: ' + result);
            }
        }
    });

    it('should free up resources properly', async() =>
    {
        if (globalObj !== undefined)
        {
            for(const funcs of ['dump',
                'test',
                'howmany',
                'multipleArgumentFunc',
                'doABadThing',
                'promiseTest',
                'addOne',
                'makeEight',
                'test1',
                'test2',
                'test3',
                'console',
                'checkValue',
                'Promise'])
            {
                await globalObj.deleteProperty(funcs);
            }
        }

        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }

        await ctx.disableTimers();
        await ctx.cleanRefs();
        await ctx.runGC();
        globalObj = undefined;
        global.gc();
        await ctx.runGC();

        const objectRefs = ctx.getObjectReferenceCount();

        if (objectRefs !== 0)
        {
            throw new Error('Object ref count is ' + objectRefs)
        }

        ctx = undefined;
        global.gc();
    });
});
