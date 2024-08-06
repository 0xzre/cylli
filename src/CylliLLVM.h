/*
    Cylli to LLVM IR Compiler
*/

#ifndef CYLLI_LLVM_H
#define CYLLI_LLVM_H

#include <string>
#include <iostream>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

class CylliLLVM
{
public:
    CylliLLVM()
    {
        moduleInit();
    }

    void exec(const std::string &cylliCode)
    {
        // 1. Parse the cylli program
        // auto ast = CylliParser::parse(cylliCode);

        // 2. Generate LLVM IR
        // auto ir = CylliLLVMGenerator::generate(ast);

        compile();

        // Debug print module
        module->print(llvm::outs(), nullptr);

        std::cout << "\n";

        // 3. Save the IR to a file
        saveModuleToFile("./output.ll");
    }

private:
    /*
        Compile ast(TODO) to LLVM IR
    */
    void compile()
    {
        // 1. Create entry main func
        fn = createFunction("main", llvm::FunctionType::get(builder->getInt32Ty(), false));

        // 2. Compile main
        auto result = generate(/* ast(TODO) */);

        // 3. Cast ret val to i32 from main
        auto i32result = builder->CreateIntCast(result, builder->getInt32Ty(), true);

        builder->CreateRet(i32result);
    }

    /*
         Main compile loop, basically interpreter at compile time
    */
    llvm::Value *generate(/* exp */)
    {
        return builder->CreateGlobalStringPtr("Henyo, World!\n");
    }

    /*
        Create a new function in the module
    */
    llvm::Function *createFunction(const std::string &fnName, llvm::FunctionType *fnType)
    {
        // Func prototype might be already present, so
        auto fn = module->getFunction(fnName);
        if (fn == nullptr)
        {
            fn = createFunctionPrototype(fnName, fnType);
        }
        createFunctionBlock(fn);
        return fn;
    }

    /*
        Create a new function prototype in the module (definition, not declaration)
    */
    llvm::Function *createFunctionPrototype(const std::string &fnName, llvm::FunctionType *fnType)
    {
        auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, *module);
        verifyFunction(*fn);
        return fn;
    }

    /*
        Create a new function block in the module
    */
    void createFunctionBlock(llvm::Function *fn)
    {
        auto entry = createBB("entry", fn);
        builder->SetInsertPoint(entry);
    }

    /*
         Create a new basic block in the module. Passing 'fn' is optional and would automtically append the block to the parent func. If not passed, it is usable be appending manually via fn->getBasicBlockList().push_back(bb)
    */
    llvm::BasicBlock *createBB(std::string name, llvm::Function *fn = nullptr)
    {
        return llvm::BasicBlock::Create(*ctx, name, fn);
    }

    /*
        Save IR to a file
    */
    void saveModuleToFile(const std::string &filename)
    {
        std::error_code errorCode;
        llvm::raw_fd_ostream dest(filename, errorCode);
        module->print(dest, nullptr);
    }
    /*
        Initialize the Cylli Module
    */
    void moduleInit()
    {
        // Open new ctx & module
        ctx = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("CylliLLVM", *ctx);

        // Create a new IRBuilder based on the ctx
        builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    }
    /*
        Current compiling function
    */
    llvm::Function *fn;
    /*
        Global LLVM Context
        Owns and manage core's "global" data from LLVM infrastrucure,
        including the type and constant uniquing tables.
     */
    std::unique_ptr<llvm::LLVMContext> ctx;

    /*
        Cylli LLVM Module
        It is the top level main container of all other LLVM IR. Each contain list of globals vars, list of funcs, list of libs for other modules.
        It depends on symbol table and various data about target characteristics for the metadata
        Module maintain GlobalList object used for holding all constant refs to global vars in the module. Means if a global vars is destroyed it will have no entries oon the GlobalList
    */
    std::unique_ptr<llvm::Module> module;

    /*
         IR Builder
         It is used to construct an LLVM IR instruction. It is used to create instructions that are added to basic blocks at end of BasicBlock or at specific location in a block.
    */
    std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif