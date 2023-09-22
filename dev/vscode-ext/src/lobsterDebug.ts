import {
    DebugSession
} from '@vscode/debugadapter';
import { DebugProtocol } from '@vscode/debugprotocol';

//TODO this should be moved outside vscode extension.

export class LobsterDebugSession extends DebugSession {
    protected launchRequest(
        response: DebugProtocol.LaunchResponse, 
        args: DebugProtocol.LaunchRequestArguments, 
        request?: DebugProtocol.Request
    ): void {
        console.log("Whoa");
    }
}