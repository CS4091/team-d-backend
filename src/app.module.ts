import { MiddlewareConsumer, Module, NestModule } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { AssetsModule } from './assets/assets.module';
import { AuthMiddleware } from './auth/auth.middleware';
import { AuthModule, DATA_SOURCE, PREFIX } from './auth/auth.module';
import { AviationModule } from './aviation/aviation.module';
import { DBModule } from './db/db.module';
import { OrgsModule } from './orgs/orgs.module';
import { UsersModule } from './users/users.module';
import { UsersService } from './users/users.service';
import { serveClient } from './utils/utils';

@Module({
	imports: [DBModule, UsersModule, AviationModule, OrgsModule, AssetsModule, AuthModule.register({ prefix: 'arro' }), ...serveClient()],
	controllers: [AppController],
	providers: [AppService, { provide: PREFIX, useValue: 'placeholder' }, { provide: DATA_SOURCE, useClass: UsersService }]
})
export class AppModule implements NestModule {
	public configure(consumer: MiddlewareConsumer): void {
		consumer.apply(AuthMiddleware).forRoutes('*');
	}
}

