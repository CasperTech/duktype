import 'mocha';
import { Context, ObjectScope } from '../../index';
import * as uuid from 'uuid'
import Done = Mocha.Done;

let ctx: Context | undefined = new Context();
let globalObj: ObjectScope | undefined;
let response = '';

describe('Context', () =>
{
    it('should instantiate properly', (done: Done) =>
    {
        try
        {
            if (ctx instanceof Context)
            {
                done();
            }
            else
            {
                done(new Error('Context is not defined properly'));
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should evaluate a simple numeric expression', (done: Done) =>
    {
        try
        {
            if (ctx === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = ctx.eval('4*4/2');
            if (result === 8)
            {
                done();
            }
            else
            {
                done(new Error())
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should give a valid global object', (done: Done) =>
    {
        try
        {
            if (ctx === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj = ctx.getGlobalObject();

            const result = JSON.parse(ctx.eval('JSON.stringify(this)'));
            if (result.performance !== undefined)
            {
                done();
            }
            else
            {
                done(new Error());
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should allow a method to be created on the global object', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj.setProperty('dump', (str: string) =>
            {
                response = str;
            });

            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should allow the method to be called by the context', (done: Done) =>
    {
        try
        {
            const testID = uuid.v4();
            if (ctx === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            ctx.eval('dump(\'' + testID + '\');')

            if (response === testID)
            {
                done();
            }
            else
            {
                done(new Error());
            }
        }
        catch(err)
        {
            done(err);
        }
    });
});

describe('Global Object', () =>
{
    it('should allow the method to be called directly on the global object', (done: Done) =>
    {
        try
        {
            const testID = uuid.v4();
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj.callMethod('dump', testID)

            if (response === testID)
            {
                done();
            }
            else
            {
                done(new Error());
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should allow a property to be set', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj.setProperty('test', 'testValue');
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should allow a property value to be retrieved', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            if (globalObj.getProperty('test') === 'testValue')
            {
                done();
            }
            else
            {
                done(new Error());
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should allow a property value to be deleted', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj.deleteProperty('test');
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should return undefined when getting a deleted property', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            if (globalObj.getProperty('test') === undefined)
            {
                done();
            }
            else
            {
                done(new Error());
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    const testFunctionRet = uuid.v4();
    it('should allow a function property to be set', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj.setProperty('test', (seed: string) => {
                return testFunctionRet + seed;
            });

            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should allow a function property to be called', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj.callMethod('test');
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should get the correct return value from the function property', (done: Done) =>
    {
        try
        {
            const seed = uuid.v4();
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            if (globalObj.callMethod('test', seed) === testFunctionRet + seed)
            {
                done();
            }
            else
            {
                done(new Error());
            }
        }
        catch(err)
        {
            done(err);
        }
    });
});

let childObject: ObjectScope | undefined;

describe('ObjectScope', () =>
{
    it('should allow a child object to be created ', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            childObject = globalObj.createObject('foo');
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should contain the newly created child object', (done: Done) =>
    {
        try
        {
            if (ctx === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = JSON.parse(ctx.eval('JSON.stringify(this)'));
            if (result.foo !== undefined)
            {
                done();
            }
            else
            {
                done(new Error());
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    let furtherChild: ObjectScope | undefined;
    it('should allow a further child to be created on that child', (done: Done) =>
    {
        try
        {
            if (childObject !== undefined)
            {
                furtherChild = childObject.createObject('bar');
                done();
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should not show the sub-child on the global object', (done: Done) =>
    {
        try
        {
            if (ctx === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = JSON.parse(ctx.eval('JSON.stringify(this)'));
            if (result.bar === undefined)
            {
                done();
            }
            else
            {
                done(new Error());
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should show the sub-child on the first child object', (done: Done) =>
    {
        try
        {
            if (ctx === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = JSON.parse(ctx.eval('JSON.stringify(this.foo)'));
            if (result.bar !== undefined)
            {
                done();
            }
            else
            {
                done(new Error());
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should be able to access a method on the global object from the second child', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj.setProperty('howmany', () => {
                return 70;
            })
            if (ctx === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            ctx.eval(`
                foo.bar.count = function(start) {
                    return start + howmany();
                };
            `);
            if (furtherChild !== undefined)
            {
                const result = furtherChild.callMethod('count', 12);
                if (result === 82)
                {
                    done();
                }
                else
                {
                    done(new Error());
                }
            }
        }
        catch(err)
        {
            console.error(err);
            done(err);
        }
    });

    it('should be able to pass functions with any number of arguments', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj.setProperty('multipleArgumentFunc', (...args: any[]) => {
                return args.length;
            })

            const result = globalObj.callMethod('multipleArgumentFunc', 'hello', 42, new Date(), Buffer.alloc(4), {hello: 'test'}, new Error('the cake is a lie'));
            if (result === 6)
            {
                done();
            }
            else
            {
                done(new Error(result + ' arguments received'));
            }
        }
        catch(err)
        {
            console.error(err);
            done(err);
        }
    });

    it('should still be able to access the deleted object due to reference', (done: Done) =>
    {
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            globalObj.deleteProperty('foo');
            if (furtherChild !== undefined)
            {
                const result = furtherChild.callMethod('count', 12);
                if (result === 82)
                {
                    done()
                }
                else
                {
                    done(new Error())
                }
            }
        }
        catch(err)
        {
            done(err);
        }
    });

    it('objects should be garbage collected properly', (done: Done) =>
    {
        childObject = undefined;
        furtherChild = undefined;
        global.gc();
        if (ctx === undefined)
        {
            return done(new Error('Context is undefined'));
        }
        ctx.runGC();
        const objectRefs = ctx.getObjectReferenceCount();
        if (objectRefs === 0)
        {
            done();
        }
        else
        {
            done(new Error('Object ref count is ' + objectRefs));
        }
    });
});

describe('Buffers', () =>
{
    if (ctx === undefined)
    {
        throw new Error('Context is undefined');
    }
    ctx.eval(`
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
    it('should pass a Node.JS buffer object with data intact and back again', (done: Done) =>
    {
        const buf = Buffer.from([1,2,3,4,5]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            if (!(result.value instanceof Buffer) || result.construct !== 'Buffer')
            {
                return done(new Error('Returned value is not a buffer (its a ' + result.value.constructor.name + ')'));
            }
            if (Buffer.compare(buf, result.value) !== 0)
            {
                return done(new Error('Buffers are not the same (compare)'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed.data[x])
                {
                    return done(new Error('Buffers are not the same (manual)'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });
});
describe('Primitive Types', () =>
{
    describe('Boolean', () =>
    {
        it('should pass with value and type intact and back again', (done: Done) =>
        {
            let val = true;
            try
            {
                if (globalObj === undefined)
                {
                    return done(new Error('Context is undefined'));
                }
                let result = globalObj.callMethod('checkValue', val);
                if (result.value === undefined)
                {
                    return done(new Error('Value is undefined'));
                }
                if (typeof result.value !== 'boolean')
                {
                    return done(new Error('Value is not a boolean'));
                }
                if (result.type !== 'boolean')
                {
                    return done(new Error('Type is not a boolean'));
                }
                if (result.value !== val)
                {
                    return done(new Error('Returned value differs from transmitted value'));
                }

                val = false;
                result = globalObj.callMethod('checkValue', val);
                if (result.value === undefined)
                {
                    return done(new Error('Value is undefined'));
                }
                if (typeof result.value !== 'boolean')
                {
                    return done(new Error('Value is not a boolean'));
                }
                if (result.type !== 'boolean')
                {
                    return done(new Error('Type is not a boolean'));
                }
                if (result.value !== val)
                {
                    return done(new Error('Returned value differs from transmitted value'));
                }

                done();
            }
            catch (err)
            {
                done(err);
            }
        });
    });
    describe('Number', () =>
    {
        it('should pass with value and type intact and back again', (done: Done) =>
        {
            let val = 0.192809;
            try
            {
                if (globalObj === undefined)
                {
                    return done(new Error('Context is undefined'));
                }
                let result = globalObj.callMethod('checkValue', val);
                if (result.value === undefined)
                {
                    return done(new Error('Value is undefined'));
                }
                if (typeof result.value !== 'number')
                {
                    return done(new Error('Value is not a number'));
                }
                if (result.type !== 'number')
                {
                    return done(new Error('Type is not a number'));
                }
                if (result.value !== val)
                {
                    return done(new Error('Returned value differs from transmitted value'));
                }
                done();
            }
            catch (err)
            {
                done(err);
            }
        });
    });
    describe('UTF-8 String', () =>
    {
        it('should pass with value and type intact and back again', (done: Done) =>
        {
            let val = `An preost wes on leoden, Laȝamon was ihoten. He wes Leovenaðes sone -- 
                       liðe him be Drihten. He wonede at Ernleȝe at æðelen are chirechen, 
                       Uppen Sevarne staþe, sel þar him þuhte, Onfest Radestone, þer he bock radde.`;
            try
            {
                if (globalObj === undefined)
                {
                    return done(new Error('Context is undefined'));
                }
                let result = globalObj.callMethod('checkValue', val);
                if (result.value === undefined)
                {
                    return done(new Error('Value is undefined'));
                }
                if (typeof result.value !== 'string')
                {
                    return done(new Error('Value is not a string'));
                }
                if (result.type !== 'string')
                {
                    return done(new Error('Type is not a string'));
                }
                if (result.value !== val)
                {
                    return done(new Error('Returned value differs from transmitted value'));
                }
                done();
            }
            catch (err)
            {
                done(err);
            }
        });
    });
    describe('Undefined', () =>
    {
        it('should pass with type intact and back again', (done: Done) =>
        {
            let val = undefined;
            try
            {
                if (globalObj === undefined)
                {
                    return done(new Error('Context is undefined'));
                }
                let result = globalObj.callMethod('checkValue', val);
                if (typeof result.value !== 'undefined')
                {
                    return done(new Error('Value is not undefined'));
                }
                if (result.type !== 'undefined')
                {
                    return done(new Error('Type is not undefined'));
                }
                if (result.value !== val)
                {
                    return done(new Error('Returned value differs from transmitted value'));
                }
                done();
            }
            catch (err)
            {
                done(err);
            }
        });
    });
    describe('Null', () =>
    {
        it('should pass with type intact and back again', (done: Done) =>
        {
            let val = null;
            try
            {
                if (globalObj === undefined)
                {
                    return done(new Error('Context is undefined'));
                }
                let result = globalObj.callMethod('checkValue', val);
                if (typeof result.value !== 'object')
                {
                    return done(new Error('Value is not object'));
                }
                if (result.type !== 'object')
                {
                    return done(new Error('Type is not object'));
                }
                if (result.value !== val)
                {
                    return done(new Error('Returned value differs from transmitted value'));
                }
                done();
            }
            catch (err)
            {
                done(err);
            }
        });
    });
});
describe('Arrays', () =>
{
    it('should pass an array with data intact and back again', (done: Done) =>
    {
        const buf = [0, 291, -220, 0.329192, 'hello', ['anotherarray', {key: 'value'}], {key: 'value'}, true, false]
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an array'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            if (!(result.value instanceof Array) || result.construct !== 'Array')
            {
                return done(new Error('Returned value is not an Array (its a ' + result.value.constructor.name + ')'));
            }
            if (JSON.stringify(buf) !== JSON.stringify(result.value))
            {
                return done(new Error('Returned value differs from duktape stringified value'));
            }
            if (JSON.stringify(buf) !== JSON.stringify(JSON.parse(result.stringified)))
            {
                return done(new Error('Node stringified value differs from duktape re-stringified value'));
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });
});
describe('TypedArrays', () =>
{
    it('should pass a Uint8ClampedArray with data intact and back again', (done: Done) =>
    {
        const buf = Uint8ClampedArray.from([1,2,3,4,5]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            if (!(result.value instanceof Uint8ClampedArray) || result.construct !== 'Uint8ClampedArray')
            {
                return done(new Error('Returned value is not a Uint8ClampedArray (its a ' + result.value.constructor.name + ')'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
                {
                    return done(new Error('Arrays do not match'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass a Uint8Array with data intact and back again', (done: Done) =>
    {
        const buf = Uint8Array.from([1,2,3,4,5]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            if (!(result.value instanceof Uint8Array)  || result.construct !== 'Uint8Array')
            {
                return done(new Error('Returned value is not a UInt8Array (its a ' + result.value.constructor.name + ')'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
                {
                    return done(new Error('Arrays do not match'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass a Uint16Array with data intact and back again', (done: Done) =>
    {
        const buf = Uint16Array.from([1,2,3,4,5]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            if (!(result.value instanceof Uint16Array)  || result.construct !== 'Uint16Array')
            {
                return done(new Error('Returned value is not a Uint16Array (its a ' + result.value.constructor.name + ')'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
                {
                    return done(new Error('Arrays do not match'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass a Uint32Array with data intact and back again', (done: Done) =>
    {
        const buf = Uint32Array.from([1,2,3,4,5]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            if (!(result.value instanceof Uint32Array)  || result.construct !== 'Uint32Array')
            {
                return done(new Error('Returned value is not a Uint32Array (its a ' + result.value.constructor.name + ')'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
                {
                    return done(new Error('Arrays do not match'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass a Int8Array with data intact and back again', (done: Done) =>
    {
        const buf = Int8Array.from([-1,2,-3,4,-5]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (!(result.value instanceof Int8Array)  || result.construct !== 'Int8Array')
            {
                return done(new Error('Returned value is not an Int8Array (its a ' + result.value.constructor.name + ')'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
                {
                    return done(new Error('Arrays do not match'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass a Int16Array with data intact and back again', (done: Done) =>
    {
        const buf = Int16Array.from([-1,2,-3,4,-5]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (!(result.value instanceof Int16Array)  || result.construct !== 'Int16Array')
            {
                return done(new Error('Returned value is not an Int16Array (its a ' + result.value.constructor.name + ')'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
                {
                    return done(new Error('Arrays do not match'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass a Int32Array with data intact and back again', (done: Done) =>
    {
        const buf = Int32Array.from([-1,2,-3,4,-5]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (!(result.value instanceof Int32Array) || result.construct !== 'Int32Array')
            {
                return done(new Error('Returned value is not an Int32Array (its a ' + result.value.constructor.name + ')'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
                {
                    return done(new Error('Arrays do not match'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass a Float32Array with data intact and back again', (done: Done) =>
    {
        const buf = Float32Array.from([-1192.19,222.212,-3.1091,429192.021,3.14159265358979323846264338327950288419716939937510]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (!(result.value instanceof Float32Array) || result.construct !== 'Float32Array')
            {
                return done(new Error('Returned value is not a Float32Array (its a ' + result.value.constructor.name + ')'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
                {
                    return done(new Error('Arrays do not match'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass a Float64Array with data intact and back again', (done: Done) =>
    {
        const buf = Float64Array.from([-1192.19,222.212,-3.1091,429192.021,3.14159265358979323846264338327950288419716939937510]);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (!(result.value instanceof Float64Array) || result.construct !== 'Float64Array')
            {
                return done(new Error('Returned value is not a Float64Array (its a ' + result.value.constructor.name + ')'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            const parsed = JSON.parse(result.stringified);
            for (let x = 0; x < buf.length; x++)
            {
                if (buf[x] !== result.value[x] || buf[x] !== parsed[x])
                {
                    return done(new Error('Arrays do not match'));
                }
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass an ArrayBuffer with data intact and back again', (done: Done) =>
    {
        const buf = new ArrayBuffer(8);
        const dView = new DataView(buf);
        // @ts-ignore
        dView.setBigInt64(0, 9121312131814171817n, true);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (!(result.value instanceof ArrayBuffer) || result.construct !== 'ArrayBuffer')
            {
                return done(new Error('Returned value is not an ArrayBuffer (its a ' + result.value.constructor.name + ')'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            const secondDView = new DataView(result.value);
            // @ts-ignore
            if (secondDView.getBigInt64(0, true) !== 9121312131814171817n)
            {
                return done(new Error('Arrays do not match'));
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should pass a DataView with data intact and back again', (done: Done) =>
    {
        const b = new ArrayBuffer(8);
        const buf = new DataView(b);
        // @ts-ignore
        buf.setBigInt64(0, 9121312131814171817n, true);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', buf);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (!(result.value instanceof DataView) || result.construct !== 'DataView')
            {
                return done(new Error('Returned value is not a DataView (its a ' + result.value.constructor.name + ')'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            // @ts-ignore
            if (result.value.getBigInt64(0, true) !== 9121312131814171817n)
            {
                return done(new Error('Arrays do not match'));
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });
});
describe('Dates', () =>
{
    it('should pass a Date object with data intact and back again', (done: Done) =>
    {
        const val = new Date(new Date().getTime() - 2764800000);
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', val);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            if (!(result.value instanceof Date) || result.construct !== 'Date')
            {
                return done(new Error('Returned value is not a date (its a ' + result.value.constructor.name + ')'));
            }
            if (val.getTime() !== result.value.getTime())
            {
                return done(new Error('Dates are not the same (getTime)'));
            }
            if (val.toISOString() !== result.value.toISOString())
            {
                return done(new Error('Dates are not the same (ISOString)'));
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });
});
describe('Errors', () =>
{
    it('should pass an Error object with message intact and back again', (done: Done) =>
    {
        const val = new Error(uuid.v4());
        try
        {
            if (globalObj === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            const result = globalObj.callMethod('checkValue', val);
            if (result.value === undefined)
            {
                return done(new Error('Value is undefined'));
            }
            if (typeof result.value !== 'object')
            {
                return done(new Error('Value is not an object'));
            }
            if (result.type !== 'object')
            {
                return done(new Error('Type is not an object'));
            }
            if (!(result.value instanceof Error) || result.construct !== 'Error')
            {
                return done(new Error('Returned value is not an Error (its a ' + result.value.constructor.name + ')'));
            }
            if ('Uncaught Error: ' + val.message !== result.value.message)
            {
                return done(new Error('Messages are not the same (' + val.message + ' vs ' + result.value.message + ')'));
            }
            done();
        }
        catch(err)
        {
            done(err);
        }
    });

    it('should be possible to catch a thrown exception', (done: Done) =>
    {
        if (globalObj === undefined)
        {
            return done(new Error('Context is undefined'));
        }
        globalObj.setProperty('doABadThing', () => {
            throw new Error('A bad thing happened');
        });

        try
        {
            globalObj.callMethod('doABadThing');
            done(new Error('An error was not thrown'));
        }
        catch(error)
        {
            if (error.message !== 'Error: Uncaught Error: A bad thing happened')
            {
                return done(new Error('Unexpected error message caught'));
            }
            done();
        }
    });

    it('should be possible for duktape to catch an exception thrown in node', (done: Done) =>
    {
        if (ctx === undefined)
        {
            return done(new Error('Context is undefined'));
        }
        const result = ctx.eval(`(function () {
            try { doABadThing(); } catch (error) { return error.message }
        })()`)
        if (result === 'Uncaught Error: A bad thing happened')
        {
            done();
        }
        else
        {
            done(new Error())
        }
    });
});
describe('Promises and Timers', () =>
{
    it('should enable promises and timers successfully', (done: Done) =>
    {
        try
        {
            if (ctx === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            ctx.enablePromises();
            done();
        }
        catch(error)
        {
            done(error);
        }
    });
    it('should delay eval by at least 100ms', async () =>
    {
        const d = new Date().getTime();
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        if (globalObj === undefined)
        {
            throw new Error('GlobalObj is undefined');
        }
        const prom = ctx.eval(`
            new Promise(function (resolve, reject) {
                setTimeout(function(){
                    resolve();                
                }, 100);
            });
        `);
        await prom;
        const time = (new Date().getTime() - d);
        if (time >= 100)
        {
            return
        }
        else
        {
            throw new Error('Time in promise less than 100ms: ' + time);
        }
    });
    it('should pass promises in both directions', async () =>
    {
        if (globalObj === undefined)
        {
            throw new Error('Context is undefined');
        }
        globalObj.setProperty('promiseTest', async() => {
            return 'promiseWorked'
        });
        if (ctx === undefined)
        {
            throw new Error('Context is undefined');
        }
        const result = await ctx.eval(`new Promise(function(resolve, reject){ promiseTest().then(function (result){ resolve(result);  }) });`)
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
describe('Garbage Collection', () =>
{
    it('should run successfully', (done: Done) =>
    {
        try
        {
            if (ctx === undefined)
            {
                return done(new Error('Context is undefined'));
            }
            ctx.runGC();
            done();
        }
        catch(error)
        {
            done(error);
        }
    });
});

describe('Complex operations', async() =>
{
    it('should stack calls without issue', async() =>
    {
        if (globalObj === undefined)
        {
            throw new Error('Context is undefined');
        }
        globalObj.setProperty('addOne', (num: number) =>
        {
            return num + 1;
        })
        globalObj.setProperty('makeEight', async () =>
        {
            if (globalObj === undefined)
            {
                throw new Error('Context is undefined');
            }
            return await globalObj.callMethod('addOne', await globalObj.callMethod('addOne', await globalObj.callMethod('addOne', await globalObj.callMethod('addOne', 4))));
        });
        const result = await globalObj.callMethod('makeEight');
        if (result !== 8)
        {
            throw new Error('Unexpected return value: ' + result);
        }
    });

    it('should handle multiple simultaneous calls', async() =>
    {
        const promises: Promise<number>[] = [];
        if (globalObj === undefined)
        {
            throw new Error('Context is undefined');
        }
        for (let x = 0; x < 100; x++)
        {
            promises.push(globalObj.callMethod('makeEight'));
        }
        for (const promise of promises)
        {
            const result = await promise;
            if (result !== 8)
            {
                throw new Error('Invalid result received');
            }
        }
    });

    it('should free up resources properly', async() =>
    {
        if (ctx !== undefined)
        {
            ctx.disableTimers();
            ctx.runGC();
        }
        globalObj = undefined;
        ctx = undefined;
        global.gc();
    });
});
