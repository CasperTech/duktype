import { TypescriptCompiler as compiler } from './ts-src/TypescriptCompiler';

declare namespace duktype
{
    export class TypescriptCompiler extends compiler
    {

    }

    export class Context
    {
        constructor();
        getGlobalObject(): ObjectScope;
        eval(script: string): any;
        enablePromises(): void;
        enableTimers(): void;
        disableTimers(): void;
        runGC(): void;
        getObjectReferenceCount(): number;
    }

    export class ObjectScope
    {
        setProperty(name: string, value: any): void;
        getProperty(name: string): any;
        deleteProperty(name: string): void;
        callMethod(name: string, ...args: any[]): any;
        createObject(name: string): ObjectScope;
    }

    export class AsyncContext
    {
        constructor();
        getGlobalObject(): AsyncObjectScope;
        eval(script: string): Promise<any>;
        enablePromises(): Promise<void>;
        enableTimers(): void;
        disableTimers(): void;
        runGC(): Promise<void>;
        getObjectReferenceCount(): number;
    }

    export class AsyncObjectScope
    {
        setProperty(name: string, value: any): Promise<void>;
        getProperty(name: string): Promise<any>;
        deleteProperty(name: string): Promise<void>;
        callMethod(name: string, ...args: any[]): Promise<any>;
        createObject(name: string): Promise<AsyncObjectScope>;
    }

    export class DukCallback
    {
        call(...args: any[]): any;
    }

    export class DukAsyncCallback
    {
        call(...args: any[]): any;
    }
}

export = duktype;