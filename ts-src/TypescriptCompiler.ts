import * as ts from 'typescript';
import * as fs from 'fs';
import * as pathResolver from 'path';
import { CompilerOptions, ModuleKind, ScriptTarget, SourceFile } from 'typescript';

export class TypescriptCompiler
{
    private options: CompilerOptions = {
        target: ScriptTarget.ES5,
        module: ModuleKind.None
    }

    lib: string;

    constructor()
    {
        this.lib = fs.readFileSync(pathResolver.resolve(__dirname + '/../lib.duktype.d.ts')).toString('utf-8');
    }

    public compile(source: string): Promise<string>
    {
        return new Promise<string>((resolve, reject) =>
        {
            const compilerHost = ts.createCompilerHost(this.options);
            compilerHost.getDefaultLibFileName = (options: CompilerOptions): string =>
            {
                return 'lib.duktype.d.ts';
            };
            compilerHost.getSourceFile = (fileName: string, languageVersion: ScriptTarget): SourceFile =>
            {
                if (fileName == 'source.ts')
                {
                    return ts.createSourceFile(fileName, source, ts.ScriptTarget.ES5, true);
                }
                else if (fileName == 'lib.duktype.d.ts')
                {
                    return ts.createSourceFile(fileName, this.lib, ts.ScriptTarget.ES5, true);
                }
                else
                {
                    return ts.createSourceFile(fileName, '', ts.ScriptTarget.ES5, true);
                }
                return null as any as SourceFile;
            };
            compilerHost.writeFile = (fileName: string, contents: string) =>
            {
                resolve(contents);
            };
            const program = ts.createProgram(['source'], this.options, compilerHost);
            program.emit();
        });
    }
};