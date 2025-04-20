import { CallHandler, ExecutionContext, Injectable, NestInterceptor } from '@nestjs/common';
import { Request, Response } from 'express';
import { appendFileSync } from 'fs';
import { Observable, tap } from 'rxjs';

@Injectable()
export class LoggingInterceptor implements NestInterceptor {
	public intercept(context: ExecutionContext, next: CallHandler): Observable<any> {
		const req = context.switchToHttp().getRequest<Request>(),
			res = context.switchToHttp().getResponse<Response>();

		return next.handle().pipe(
			tap((data) => {
				const reqStr = `--- REQUEST ---\n${req.method} ${req.url}\n${Object.entries(req.headers)
					.map(([key, val]) => `${key}: ${val}`)
					.join('\n')}${req.method === 'POST' ? `\n\n${JSON.stringify(req.body, null, 4)}` : ''}`;
				const resStr = `${res.statusCode}\n${JSON.stringify(data, null, 4)}\n--- RESPONSE ---`;

				appendFileSync('server.log', reqStr + '\n\n<===============>\n\n' + resStr + '\n');
			})
		);
	}
}

