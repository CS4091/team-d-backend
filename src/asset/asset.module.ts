import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { OrgModule } from 'src/org/org.module';
import { AssetController } from './asset.controller';
import { AssetService } from './asset.service';

@Module({
	imports: [DBModule, OrgModule],
	providers: [AssetService],
	controllers: [AssetController],
	exports: [AssetService]
})
export class AssetModule {}

