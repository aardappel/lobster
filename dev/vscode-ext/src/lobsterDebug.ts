import {
    DebugSession, TerminatedEvent
} from '@vscode/debugadapter';
import { DebugProtocol } from '@vscode/debugprotocol';
import { spawn, ChildProcess } from 'child_process';
import { workspace } from 'vscode';

interface ILaunchRequestArguments extends DebugProtocol.LaunchRequestArguments {
	/** An absolute path to the "program" to debug. */
	program: string;

    params?: string[];
	/** Automatically stop target after launch. If not specified, target does not stop. */
	stopOnEntry?: boolean;
	/** run without debugging */
	noDebug?: boolean;
}


export class LobsterDebugSession extends DebugSession {
    process: ChildProcess;

    protected launchRequest(
        response: DebugProtocol.LaunchResponse, 
        args: ILaunchRequestArguments,
    ): void {
        //TODO add actual debugging
        const executable = workspace.getConfiguration('lobster').get('executable') as string;
        const imports = workspace.getConfiguration('lobster').get('imports') as string[];

        this.process = spawn(executable, [
            ...imports.map(i => "--import " + i),
            args.program,
            ...(args.params || [])
        ]);

        this.process.on('spawn', () => {
            this.sendResponse(response);
        });

        this.process.on('error', (err) => {
            this.sendErrorResponse(response, 1, "Error executing lobster: {message}", { message: err.message });
        });

        this.process.on('exit', (code, signal) => {
            this.sendEvent(new TerminatedEvent());
        });
    }
}