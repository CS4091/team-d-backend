import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { OrgsModule } from 'src/orgs/orgs.module';
import { AssetsController } from './assets.controller';
import { AssetsService } from './assets.service';

@Module({
	imports: [DBModule, OrgsModule],
	providers: [AssetsService],
	controllers: [AssetsController],
	exports: [AssetsService]
})
export class AssetsModule {}

