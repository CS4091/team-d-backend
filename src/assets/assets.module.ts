import { Module } from '@nestjs/common';
import { AviationModule } from 'src/aviation/aviation.module';
import { DBModule } from 'src/db/db.module';
import { OrgsModule } from 'src/orgs/orgs.module';
import { AssetsController } from './assets.controller';
import { AssetsService } from './assets.service';

@Module({
	imports: [DBModule, OrgsModule, AviationModule],
	providers: [AssetsService],
	controllers: [AssetsController],
	exports: [AssetsService]
})
export class AssetsModule {}

