import { createParamDecorator, ExecutionContext } from '@nestjs/common';

export const ReqUser = createParamDecorator((_, ctx: ExecutionContext) => ctx.switchToHttp().getRequest<Express.Request>().user);
