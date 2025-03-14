import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { UserModule } from 'src/user/user.module';
import { OrgController } from './org.controller';
import { OrgService } from './org.service';

@Module({
	imports: [DBModule, UserModule],
	providers: [OrgService],
	controllers: [OrgController],
	exports: [OrgService]
})
export class OrgModule {}

