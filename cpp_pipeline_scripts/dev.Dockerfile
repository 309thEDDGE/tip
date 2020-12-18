FROM registry.il2.dsop.io/skicamp/project-opal/tip/test:1.0

COPY --from=registry.il2.dsop.io/skicamp/project-opal/tip/build:MR57 /deps /deps
