import { MiddlewareConsumer, Module, NestModule } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { AssetModule } from './asset/asset.module';
import { AuthMiddleware } from './auth/auth.middleware';
import { AuthModule, DATA_SOURCE, PREFIX } from './auth/auth.module';
import { AviationModule } from './aviation/aviation.module';
import { DBModule } from './db/db.module';
import { OrgModule } from './org/org.module';
import { UserModule } from './user/user.module';
import { UserService } from './user/user.service';
import { serveClient } from './utils/utils';

@Module({
	imports: [DBModule, UserModule, AviationModule, OrgModule, AssetModule, AuthModule.register({ prefix: 'arro' }), ...serveClient()],
	controllers: [AppController],
	providers: [AppService, { provide: PREFIX, useValue: 'placeholder' }, { provide: DATA_SOURCE, useClass: UserService }]
})
export class AppModule implements NestModule {
	public configure(consumer: MiddlewareConsumer): void {
		consumer.apply(AuthMiddleware).forRoutes('*');
	}
}

