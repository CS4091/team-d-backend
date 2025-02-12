import { MiddlewareConsumer, Module, NestModule } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { AuthMiddleware } from './auth/auth.middleware';
import { AuthModule, PREFIX } from './auth/auth.module';
import { serveClient } from './utils/utils';

@Module({
	imports: [AuthModule.register({ prefix: 'placeholder' }), ...serveClient()],
	controllers: [AppController],
	providers: [AppService, { provide: PREFIX, useValue: 'placeholder' }]
})
export class AppModule implements NestModule {
	public configure(consumer: MiddlewareConsumer): void {
		consumer.apply(AuthMiddleware).forRoutes('*');
	}
}

